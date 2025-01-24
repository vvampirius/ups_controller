package main

import (
	"database/sql"
	"flag"
	"fmt"
	_ "github.com/lib/pq"
	"log"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"regexp"
	"syscall"
)

const VERSION = `0.2`

var (
	ErrorLog             = log.New(os.Stderr, `error#`, log.Lshortfile)
	DebugLog             = log.New(os.Stdout, `debug#`, log.Lshortfile)
	RebootTargetRegexp   = regexp.MustCompile(`(?m)^.*reboot.target\W+start.*$`)
	PoweroffTargetRegexp = regexp.MustCompile(`(?m)^.*poweroff.target\W+start.*$`)
)

func helpText() {
	fmt.Println(`bla-bla-bla`)
	flag.PrintDefaults()
}

func main() {
	configPath := flag.String("c", "/etc/rpi-ups-client.yml", "rpi-ups-client config path")
	help := flag.Bool("h", false, "print this help")
	ver := flag.Bool("v", false, "Show version")
	flag.Parse()

	if *help {
		helpText()
		os.Exit(0)
	}

	if *ver {
		fmt.Println(VERSION)
		os.Exit(0)
	}

	config := Config{}
	if err := config.Load(*configPath); err != nil {
		os.Exit(1)
	}

	var db *sql.DB
	if config.DbUrl != `` {
		var err error
		// "postgres://pqgotest:password@localhost/pqgotest?sslmode=require"
		if db, err = sql.Open("postgres", config.DbUrl); err != nil {
			ErrorLog.Fatalln(err.Error())
		}
	}

	core := NewCore(&config, db)

	signalChan := make(chan os.Signal, 1)
	signal.Notify(signalChan, syscall.SIGTERM)
	go func() {
		<-signalChan
		defer os.Exit(0)
		cmd := exec.Command(`/usr/bin/systemctl`, `list-jobs`)
		cmdOutput, err := cmd.Output()
		if err != nil {
			ErrorLog.Println(err.Error())
			return
		}
		if RebootTargetRegexp.Match(cmdOutput) {
			DebugLog.Println(`Exit due to reboot`)
			core.writeToSerial("REBOOT")
			return
		}
		if PoweroffTargetRegexp.Match(cmdOutput) {
			DebugLog.Println(`Exit due to poweroff`)
			core.writeToSerial("SHUTDOWN")
			return
		}
	}()

	go core.readSerialRoutine()
	go core.pingRoutine()

	http.HandleFunc(`/ping`, func(w http.ResponseWriter, _ *http.Request) {
		fmt.Fprintln(w, `PONG`)
	})

	server := http.Server{Addr: config.Listen}
	if err := server.ListenAndServe(); err != nil {
		ErrorLog.Fatalln(err.Error())
	}
}
