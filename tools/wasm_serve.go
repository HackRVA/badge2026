// wasm_serve is just a simple http file server with headers set to work with pthreads in wasm
package main

import (
	"flag"
	"fmt"
	"html/template"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"time"
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

var loginTmpl = template.Must(template.New("login").Parse(`
<!DOCTYPE html>
<html lang="en"><head><meta charset="UTF-8"><title>Login</title></head>
<body>
  {{if .Error}}<p style="color:red">{{.Error}}</p>{{end}}
  <form method="POST" action="/login">
    <label>Password: <input type="password" name="password" autofocus></label>
    <button type="submit">Log in</button>
  </form>
</body>
</html>
`))

var (
	port     = flag.String("port", "8080", "Port to serve on")
	srcDir   = flag.String("src", "", "Source directory to copy from (optional)")
	destDir  = flag.String("dir", "./build_wasm/source/", "Destination directory to copy to")
	password = flag.String("password", "", "If set, requires login with this password")
)

func main() {
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

	http.HandleFunc("/login", loginHandler)
	http.Handle("/", authMiddleware(http.HandlerFunc(fileHandler), *password))

	fmt.Printf("Serving on http://0.0.0.0:%s\n", *port)
	err := http.ListenAndServe("0.0.0.0:"+*port, nil)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error starting server: %v\n", err)
		os.Exit(1)
	}
}

func fileHandler(w http.ResponseWriter, r *http.Request) {
	fs := http.FileServer(http.Dir(*destDir))
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
}

func loginHandler(w http.ResponseWriter, r *http.Request) {
	if c, err := r.Cookie("auth"); err == nil && c.Value == *password {
		http.Redirect(w, r, "/", http.StatusSeeOther)
		return
	}
	switch r.Method {
	case http.MethodGet:
		loginTmpl.Execute(w, nil)
	case http.MethodPost:
		r.ParseForm()
		if r.Form.Get("password") == *password {
			http.SetCookie(w, &http.Cookie{
				Name:     "auth",
				Value:    *password,
				Path:     "/",
				HttpOnly: true,
				Expires:  time.Now().Add(24 * time.Hour),
			})
			http.Redirect(w, r, "/", http.StatusSeeOther)
		} else {
			loginTmpl.Execute(w, struct{ Error string }{"Invalid password"})
		}
	default:
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
	}
}

func authMiddleware(next http.Handler, password string) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if password != "" && r.URL.Path != "/login" {
			c, err := r.Cookie("auth")
			if err != nil || c.Value != password {
				http.Redirect(w, r, "/login", http.StatusSeeOther)
				return
			}
		}
		next.ServeHTTP(w, r)
	})
}
