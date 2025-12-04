package main

import (
	"errors"
	"fmt"
	"os"
	"strings"
)

/*
----------------------------------------------------------

	Small utility to embed CSS and JS into an HTML file
	and generate a C header file with the result.
	Useful to embed web UIs into microcontroller firmware.

----------------------------------------------------------

	const Reads{
	"index.html", "style.css", "script.js"
		}
	const Generates{
	"index_html.h" as "MAIN_page[]" variable
	}
	const Usage{
	1. Place "index.html", "styles.css", and "script.js" in the same directory as this Go file
	2. Run: "go run main.go"
	3. The generated "index_html.h" file will contain the embedded HTML content.
	}

----------------------------------------------------------

	var author "Bastian Roth"
	var date "04 Dec 2025"
	var version "1.0"

----------------------------------------------------------
*/
func main() {
	var (
		html, css, js []byte = nil, nil, nil
		err           error
	)

	html, err = os.ReadFile("index.html")
	if err != nil {
		fmt.Printf("Could not read index.html file due to error %v", err, "Stopping process")
		panic("Nothing to do")
	}
	merged := string(html)
	for _, s := range []string{"style.css", "styles.css", "stylesheet.css"} {
		css, err = os.ReadFile(s)
		if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				err = nil
				continue
			}
			fmt.Printf("Could not read %v file due to error %v", s, err)
			break
		}
	}
	if css == nil {
		fmt.Printf("Could not read css file")
	}
	for _, s := range []string{"script.js", "main.js", "index.js"} {
		css, err = os.ReadFile(s)
		if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				err = nil
				continue
			}
			fmt.Printf("Could not read %v file due to error %v", s, err)
			break
		}
	}
	if js == nil {
		fmt.Printf("Could not read js file")
	}
	if css == nil && js == nil {
		fmt.Println("No CSS or JS files to embed. Stopping process")
		panic("Nothing to do")
	}
	if css != nil {
		merged = strings.ReplaceAll(merged,
			"<link rel=\"stylesheet\" href=\"styles.css\">",
			fmt.Sprintf("<style>%s</style>", string(css)))
	}
	if js != nil {
		merged = strings.ReplaceAll(merged,
			"<script src=\"script.js\"></script>",
			fmt.Sprintf("<script>%s</script>", string(js)))
	}
	header := fmt.Sprintf(`const char MAIN_page[] PROGMEM = R"rawliteral(%s)rwaliteral";`, merged)
	os.WriteFile("index_html.h", []byte(header), 0644)
	return
}
