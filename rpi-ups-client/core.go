package main

import (
	"bufio"
	"bytes"
	"database/sql"
	"encoding/json"
	"fmt"
	"github.com/tarm/serial"
	"net/http"
	"os"
	"time"
)

type Core struct {
	Config     *Config
	Db         *sql.DB
	SerialPort *serial.Port

	LastVoltage        *Voltage
	VoltageAggregation VoltageAggregation
}

func (core *Core) isOnlineChanged(voltage *Voltage) bool {
	if core.LastVoltage == nil || voltage == nil {
		return false
	}
	if voltage.IsOnline != core.LastVoltage.IsOnline {
		return true
	}
	return false
}

func (core *Core) openSerialPort() error {
	if core.SerialPort != nil {
		return nil
	}
	serialConfig := &serial.Config{
		Name:        core.Config.Port.Name,
		Baud:        core.Config.Port.Baud,
		ReadTimeout: 1,
		Size:        8,
	}
	port, err := serial.OpenPort(serialConfig)
	if err != nil {
		ErrorLog.Println(err.Error())
		return err
	}
	core.SerialPort = port
	return nil
}

func (core *Core) pingRoutine() {
	for {
		if err := core.writeToSerial("PING"); err != nil {
			ErrorLog.Println(err.Error())
		}
		time.Sleep(time.Minute)
	}
}

func (core *Core) readSerialRoutine() {
	lastPrint := time.Time{}
	if core.SerialPort == nil {
		if err := core.openSerialPort(); err != nil {
			os.Exit(1)
		}
	}
	for {
		scanner := bufio.NewScanner(core.SerialPort)
		for scanner.Scan() {
			text := scanner.Text()
			if text == `PONG` {
				continue
			}
			voltage := ParseVoltage(text)
			if voltage == nil {
				DebugLog.Println(text)
				continue
			}
			if time.Now().After(lastPrint.Add(time.Minute)) {
				fmt.Printf("%.2fV ONLINE:%t Uptime: %s\n", voltage.Value, voltage.IsOnline, voltage.OnlineDuration)
				lastPrint = time.Now()
			}
			core.VoltageAggregation.Add(voltage.Value)
			isOnlineChanged := core.isOnlineChanged(voltage)
			//DebugLog.Printf("online_changed: %v, count: %d, online: %v", isOnlineChanged, core.VoltageAggregation.Count,
			//voltage.IsOnline)
			if isOnlineChanged || (core.VoltageAggregation.Count >= 10 && !voltage.IsOnline) {
				if core.Db != nil {
					if _, err := core.Db.Exec(`INSERT INTO voltage(voltage, is_online) VALUES ($1, $2)`,
						core.VoltageAggregation.Avg(), voltage.IsOnline); err != nil {
						ErrorLog.Println(err.Error())
					}
				}
			}
			if isOnlineChanged {
				buffer := bytes.NewBuffer(nil)
				if err := json.NewEncoder(buffer).Encode(voltage); err == nil {
					resp, err := http.Post(core.Config.BotApiUrl+`/api/power_outage`, `application/json`, buffer)
					if err != nil {
						ErrorLog.Println(err.Error())
					} else if resp.StatusCode != 200 {
						ErrorLog.Println(resp.Status)
					}
				} else {
					ErrorLog.Println(err.Error())
				}
			}
			if core.VoltageAggregation.Count >= 10 {
				core.VoltageAggregation.Reset()
			}
			core.LastVoltage = voltage
		}
		if err := scanner.Err(); err != nil {
			ErrorLog.Fatalln(err.Error())
		}
		time.Sleep(250 * time.Millisecond)
	}
}

func (core *Core) writeToSerial(s string) error {
	if err := core.openSerialPort(); err != nil {
		return err
	}
	if _, err := fmt.Fprintln(core.SerialPort, s); err != nil {
		ErrorLog.Println(err.Error())
		return err
	}
	return nil
}

func NewCore(config *Config, db *sql.DB) *Core {
	core := Core{
		Config: config,
		Db:     db,
	}
	return &core
}
