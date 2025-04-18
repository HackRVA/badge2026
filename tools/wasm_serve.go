// wasm_serve is just a simple http file server with headers set to work with pthreads in wasm
package main

import (
	"flag"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
)

func copyDir(src, dst string) error {
	return filepath.Walk(src, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		relPath, err := filepath.Rel(src, path)
		if err != nil {
			return err
		}

		destPath := filepath.Join(dst, relPath)

		if info.IsDir() {
			return os.MkdirAll(destPath, os.ModePerm)
		}

		srcFile, err := os.Open(path)
		if err != nil {
			return err
		}
		defer srcFile.Close()

		destFile, err := os.Create(destPath)
		if err != nil {
			return err
		}
		defer destFile.Close()

		_, err = io.Copy(destFile, srcFile)
		return err
	})
}

func main() {
	port := flag.String("port", "8080", "Port to serve on")
	srcDir := flag.String("src", "", "Source directory to copy from (optional)")
	destDir := flag.String("dir", "./build_wasm/source/", "Destination directory to copy to (used for serving)")

	flag.Parse()

	if *srcDir != "" {
		fmt.Println("Copying files...")
		err := copyDir(*srcDir, *destDir)
		if err != nil {
			fmt.Fprintf(os.Stderr, "Error copying files: %v\n", err)
			os.Exit(1)
		}
		fmt.Println("Files copied successfully.")
	} else {
		fmt.Println("No source directory specified, skipping copy.")
	}

	fs := http.FileServer(http.Dir(*destDir))

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		// let's explicitly state which files should be handled strictly
		// this way we can still load in external resources
		// e.g. an embedded youtube video
		strictFiles := []string{"/sim.html", "/badge2025_c.js", "/badge2025_c.wasm", "/badge2025_c.data", "/simulator.html"}

		for _, f := range strictFiles {
			if r.URL.Path == f {
				w.Header().Set("Cross-Origin-Opener-Policy", "same-origin")
				w.Header().Set("Cross-Origin-Embedder-Policy", "require-corp")
				break
			}
		}

		fs.ServeHTTP(w, r)
	})

	fmt.Printf("Serving on http://0.0.0.0:%s\n", *port)
	err := http.ListenAndServe("0.0.0.0:"+*port, nil)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error starting server: %v\n", err)
		os.Exit(1)
	}
}
