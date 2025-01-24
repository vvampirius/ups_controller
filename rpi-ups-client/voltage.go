package main

import (
	"regexp"
	"strconv"
	"time"
)

var (
	RegexpVoltage       = regexp.MustCompile(`.*voltage:\s(\d+)$`)
	RegexpVoltageOnline = regexp.MustCompile(`.*voltage:\s(\d+), ONLINE duration:\s(\d+)$`)
)

type Voltage struct {
	Value          float64       `json:"value"`
	IsOnline       bool          `json:"is_online"`
	OnlineDuration time.Duration `json:"online_duration"`
}

func (voltage *Voltage) ParseOnlineString(s string) bool {
	match := RegexpVoltageOnline.FindStringSubmatch(s)
	if len(match) != 3 {
		return false
	}
	intValue, err := strconv.ParseInt(match[1], 10, 32)
	if err != nil {
		ErrorLog.Println(err.Error())
		return false
	}
	voltage.Value = float64(intValue) / 100
	intValue, err = strconv.ParseInt(match[2], 10, 64)
	if err != nil {
		ErrorLog.Println(err.Error())
		return false
	}
	voltage.OnlineDuration = time.Duration(intValue) * time.Millisecond
	voltage.IsOnline = true
	return true
}

func (voltage *Voltage) ParseString(s string) bool {
	match := RegexpVoltage.FindStringSubmatch(s)
	if len(match) != 2 {
		return false
	}
	intValue, err := strconv.ParseInt(match[1], 10, 32)
	if err != nil {
		ErrorLog.Println(err.Error())
		return false
	}
	voltage.Value = float64(intValue) / 100
	voltage.OnlineDuration = 0
	voltage.IsOnline = false
	return true
}

func ParseVoltage(s string) *Voltage {
	voltage := Voltage{}
	if voltage.ParseOnlineString(s) {
		return &voltage
	}
	if voltage.ParseString(s) {
		return &voltage
	}
	return nil
}
