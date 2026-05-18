package main

import (
	"fmt"
	"os"
	"path/filepath"
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
	Run: 'go run main.go <inputDir> <outputDir>' in terminal
	Extract: generated files from "/output/folder"
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
	if len(os.Args) < 3 {
		panic("Usage: merger <inputDir> <outputDir>")
	}
	input := os.Args[1]
	output := os.Args[2]

	entries, err := os.ReadDir(input)
	if err != nil {
		panic("Could not read input directory: " + err.Error())
	}

	hasFiles := false
	hasDirs := false

	for _, e := range entries {
		if e.IsDir() {
			hasDirs = true
		} else {
			hasFiles = true
		}
	}

	switch {
	case hasFiles:
		fmt.Printf("[1] Processing single project: %s \n", input)
		processProject(input, output)

	case hasDirs:
		fmt.Printf("[%d] Processing multiple projects in: %s \n", len(entries), input)
		for _, e := range entries {
			if e.IsDir() {
				projectName := e.Name()
				inPath := filepath.Join(input, projectName)
				outPath := filepath.Join(output, projectName)
				processProject(inPath, outPath)
			}
		}

	default:
		panic("[0] Input directory is empty")
	}
}

func processProject(inPath, outPath string) {
	fmt.Printf("[x] Project: %s -> %s \n", inPath, outPath)
	os.MkdirAll(outPath, os.ModePerm)

	var cpp, html, css, js []byte
	var err error

	files, err := os.ReadDir(inPath)
	if err != nil {
		fmt.Printf("Could not read project folder %s: %v\n", inPath, err)
		return
	}

	for _, f := range files {
		if f.IsDir() {
			continue
		}

		name := f.Name()
		full := filepath.Join(inPath, name)
		ext := strings.ToLower(filepath.Ext(name))

		switch ext {
		case ".cpp":
			cpp, err = os.ReadFile(full)
		case ".html":
			html, err = os.ReadFile(full)
		case ".css":
			css, err = os.ReadFile(full)
		case ".js":
			js, err = os.ReadFile(full)
		}

		if err != nil {
			fmt.Printf("Error reading %s: %v\n", full, err)
		}
	}

	if html == nil {
		fmt.Printf("Skipping %s — no index.html found\n", inPath)
		return
	}

	merged := string(html)

	if css != nil {
		reCSS := regexp.MustCompile(`<link[^>]*href="style.css"[^>]*>`)
		merged = reCSS.ReplaceAllString(merged,
			"<style>\n"+strings.TrimSpace(string(css))+"\n</style>")
	}

	if js != nil {
		safeJS := strings.ReplaceAll(string(js), "$", "$$")
		reJS := regexp.MustCompile(`<script[^>]*src="main.js"[^>]*></script>`)
		merged = reJS.ReplaceAllString(merged,
			"<script>\n"+strings.TrimSpace(string(safeJS))+"\n</script>")
	}

	if cpp != nil {
		os.WriteFile(filepath.Join(outPath, "main.ino"), cpp, 0644)
	}

	header := `const char MAIN_page[] PROGMEM = R"rawliteral(` + merged + `)rawliteral";`
	os.WriteFile(filepath.Join(outPath, "index_html.h"), []byte(header), 0644)

	fmt.Printf("Done: %s\n", inPath)
}
