package main

import (
	"gopkg.in/yaml.v3"
	"os"
)

type Config struct {
	Listen string `yaml:"listen"`
	Port   struct {
		Name string `yaml:"name"`
		Baud int    `yaml:"baud"`
	} `yaml:"port"`
	DbUrl     string `yaml:"db_url"`
	BotApiUrl string `yaml:"bot_api_url"`
}

func (config *Config) Load(path string) error {
	f, err := os.Open(path)
	if err != nil {
		ErrorLog.Println(err.Error())
		return err
	}
	defer f.Close()
	if err := yaml.NewDecoder(f).Decode(config); err != nil {
		ErrorLog.Println(err.Error())
		return err
	}
	return nil
}
