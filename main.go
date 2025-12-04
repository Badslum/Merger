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
		cpp, html, css, js []byte = nil, nil, nil, nil
		err                error
	)
	if c, err := os.ReadDir("input"); err != nil || len(c) == 0 {
		if err == nil {
			err = errors.New("input directory is empty")
		}
		fmt.Printf("Could not read input directory due to error %v %v", err, "Stopping process")
		panic("Nothing to do")
	}
	if _, err := os.ReadDir("output"); err != nil {
		os.MkdirAll("output", os.ModePerm)
		err = nil
	}
	for _, s := range []string{"main.c", "main.cpp", "firmware.c", "firmware.cpp", "sketch.c", "sketch.cpp"} {
		cpp, err = os.ReadFile("input/" + s)
		if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				err = nil
				continue
			} else {
				fmt.Printf("Could not read %v file due to error %v", s, err)
			}
		}
	}
	if cpp == nil {
		fmt.Printf("No cpp file, skipping")
	} else {
		os.WriteFile("main.cpp", cpp, 0644)
	}
	html, err = os.ReadFile("input/index.html")
	if err != nil {
		fmt.Printf("Could not read index.html file due to error %v %v", err, "Stopping process")
		panic("Nothing to do")
	}
	merged := string(html)
	for _, s := range []string{"style.css", "styles.css", "stylesheet.css"} {
		css, err = os.ReadFile("input/" + s)
		if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				err = nil
				continue
			}
			fmt.Printf("Could not read %v file due to error %v", s, err)
			break
		}
	}
	if css != nil {
		merged = strings.ReplaceAll(merged,
			"<link rel=\"stylesheet\" href=\"styles.css\">",
			fmt.Sprintf("<style>%s</style>", string(css)))
	}
	for _, s := range []string{"script.js", "main.js", "index.js"} {
		js, err = os.ReadFile("input/" + s)
		if err != nil {
			if errors.Is(err, os.ErrNotExist) {
				err = nil
				continue
			}
			fmt.Printf("Could not read %v file due to error %v", s, err)
			break
		}
	}
	if js != nil {
		merged = strings.ReplaceAll(merged,
			"<script src=\"script.js\"></script>",
			fmt.Sprintf("<script>%s</script>", string(js)))
	}
	header := fmt.Sprintf(`const char MAIN_page[] PROGMEM = R"rawliteral(%s)rwaliteral";`, merged)
	os.WriteFile("output/index_html.h", []byte(header), 0644)
}
