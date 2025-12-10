package main

import (
	"errors"
	"fmt"
	"os"
	"regexp"
	"strings"
)

/*
----------------------------------------------------------

	Small utility to embed CSS and JS into an HTML file
	and generate a C header file with the result.
	Useful to embed web UIs into microcontroller firmware.

----------------------------------------------------------

	const Reads(
	"input/*"
	)
	const Generates(
	"output/index_html.h" as "MAIN_page[]" variable,
	"output/main.cpp" when "input/*.cpp" exists
	)
	const Usage(
	Place: html, css, and js files in "/input"
	Run: "go run main.go" in terminal
	Extract: generated files from "/output"
	)

----------------------------------------------------------

	var(
	author 	"Bastian Roth"
	date 	"04 Dec 2025"
	version "1.0"
	license "MIT"
	)

----------------------------------------------------------
*/
func main() {
	var (
		cpp, html, css, js []byte = nil, nil, nil, nil
		content            []string
		err                error
	)
	if files, err := os.ReadDir("input"); err != nil || len(files) == 0 {
		if err == nil {
			err = errors.New("input directory is empty")
		}
		fmt.Printf("Could not read input directory due to error %v %v", err, "Stopping process")
		panic("Nothing to do")
	} else {
		for _, file := range files {
			fmt.Printf("Found input file: %v\n", file.Name())
			content = append(content, file.Name())
		}
	}
	if _, err := os.ReadDir("output"); err != nil {
		os.MkdirAll("output", os.ModePerm)
		err = nil
	}
	for _, s := range content {
		switch ext := strings.ToLower(s[strings.LastIndex(s, "."):]); ext {
		case ".cpp":
			cpp, err = os.ReadFile("input/" + s)
			if err != nil {
				fmt.Printf("Could not read %v file due to error %v", s, err)
			}
		case ".html":
			html, err = os.ReadFile("input/" + s)
			if err != nil {
				fmt.Printf("Could not read %v file due to error %v %v", s, err, "Stopping process")
				panic("Nothing to do")
			}
		case ".css":
			css, err = os.ReadFile("input/" + s)
			if err != nil {
				fmt.Printf("Could not read %v file due to error %v", s, err)
			}
		case ".js":
			js, err = os.ReadFile("input/" + s)
			if err != nil {
				fmt.Printf("Could not read %v file due to error %v", s, err)
			}
		default:
			fmt.Printf("Unknown file type: %v, skipping\n", s)
		}
	}
	merged := string(html)
	if css != nil {
		reCSS := regexp.MustCompile(`<link[^>]*href="style.css"[^>]*>`)
		merged = reCSS.ReplaceAllString(merged, "<style>\n"+strings.TrimSpace(string(css))+"\n</style>")
	}
	if js != nil {
		reJS := regexp.MustCompile(`<script[^>]*src="main.js"[^>]*></script>`)
		merged = reJS.ReplaceAllString(merged, "<script>\n"+strings.TrimSpace(string(js))+"\n</script>")
	}
	if cpp != nil {
		var (
			psk      string = os.Getenv("psk")
			ap_psk   string = os.Getenv("ap_psk")
			replaced string = string(cpp)
		)
		if psk != "" {
			replaced = strings.ReplaceAll(replaced, `const char* psk = "pwd";`, fmt.Sprintf(`const char* psk = "%v";`, psk))
		}
		if ap_psk != "" {
			replaced = strings.ReplaceAll(replaced, `const char* ap_psk = "ap_pwd";`, fmt.Sprintf(`const char* ap_psk = "%v";`, ap_psk))
		}
		cpp = []byte(replaced)
		os.WriteFile("output/main.cpp", cpp, 0644)
	}
	header := fmt.Sprintf(`const char MAIN_page[] PROGMEM = R"rawliteral(%s)rawliteral";`, merged)
	os.WriteFile("output/index_html.h", []byte(header), 0644)
}
