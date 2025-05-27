package main

import (
	"bytes"
	"fmt"
	"html/template"
	"io"
	"io/fs"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/sirupsen/logrus"
	"github.com/yuin/goldmark"
	"github.com/yuin/goldmark/extension"
	"github.com/yuin/goldmark/renderer/html"

	chromahtml "github.com/alecthomas/chroma/v2/formatters/html"
	highlighting "github.com/yuin/goldmark-highlighting/v2"
)

type PageData struct {
	Title     string
	Content   template.HTML
	Backlinks []Backlink
}

type Link struct {
	URL  string
	Text string
}

type Preview struct {
	Title   string
	Content string
}

type Backlink struct {
	URL     string
	Title   string
	Preview Preview
}

var (
	backlinkRegex = regexp.MustCompile(`\[\[(.*?)\]\]`)
	backlinkMap   = make(map[string][]Backlink)
)

func firstPass(inputDir string) error {
	err := filepath.Walk(inputDir, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() && strings.HasSuffix(info.Name(), ".md") {
			content, err := os.ReadFile(path)
			if err != nil {
				return err
			}

			matches := backlinkRegex.FindAllStringSubmatch(string(content), -1)
			for _, match := range matches {
				if len(match) > 1 {
					linkedPage := strings.ToLower(strings.ReplaceAll(match[1], " ", "-"))
					title := strings.ToLower(strings.TrimSuffix(info.Name(), ".md"))

					backlinkMap[linkedPage] = append(backlinkMap[linkedPage], Backlink{
						URL:   fmt.Sprintf("/pages/%s/", strings.ReplaceAll(title, " ", "-")),
						Title: title,
						Preview: Preview{
							Title: title,
							// Content: "TODO -- implement content preview",
						},
					})
				}
			}
		}

		return nil
	})
	return err
}

func transformInternalLinks(markdownContent string) string {
	internalLinkRegex := regexp.MustCompile(`\[\[(.*?)\]\]`)
	return internalLinkRegex.ReplaceAllStringFunc(markdownContent, func(match string) string {
		linkText := internalLinkRegex.ReplaceAllString(match, "$1")
		linkURL := fmt.Sprintf("/pages/%s/", strings.ToLower(strings.ReplaceAll(linkText, " ", "-")))
		return fmt.Sprintf("[%s](%s)", linkText, linkURL)
	})
}

func main() {
	inputDir := "./pages"
	outputDir := "./.dist"

	if err := os.MkdirAll(outputDir, 0o755); err != nil {
		fmt.Println("Error creating output directory:", err)
		return
	}

	if err := firstPass(inputDir); err != nil {
		fmt.Println("Error in first pass:", err)
		return
	}

	var pages []string
	pageTpl, err := template.ParseFiles("templates/page.html")
	if err != nil {
		fmt.Println("Error loading page template:", err)
		return
	}

	err = filepath.Walk(inputDir, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() && strings.HasSuffix(info.Name(), ".md") {
			fileNameWithoutExt := strings.ReplaceAll(strings.TrimSuffix(info.Name(), filepath.Ext(info.Name())), " ", "-")
			generateHTMLPage(pageTpl, path, outputDir, fileNameWithoutExt)
			pages = append(pages, fileNameWithoutExt)
		}
		return nil
	})
	if err != nil {
		fmt.Println("Error walking through markdown directory:", err)
		return
	}

	os.Mkdir(".dist/assets/", 0o755)
	if err := CopyDir("./assets/", ".dist/"); err != nil {
		logrus.Error(err)
	}
	os.Mkdir(".dist/js/", 0o755)
	if err := CopyDir("./js/", ".dist/"); err != nil {
		logrus.Error(err)
	}

	os.WriteFile(".dist/CNAME", []byte("brain.dustinfirebaugh.com"), 0o755)
}

func removeYAMLFrontmatter(content string) string {
	frontmatterRegex := regexp.MustCompile(`(?ms)^---\n.*?\n---\n`)
	return frontmatterRegex.ReplaceAllString(content, "")
}

func generateHTMLPage(tpl *template.Template, inputFilePath, outputDir, fileNameWithoutExt string) {
	content, err := os.ReadFile(inputFilePath)
	if err != nil {
		fmt.Println("Error reading markdown file:", err)
		return
	}

	contentNoFrontmatter := removeYAMLFrontmatter(string(content))
	processedContent := transformInternalLinks(contentNoFrontmatter)

	markdown := goldmark.New(
		goldmark.WithRendererOptions(
			html.WithXHTML(),
			html.WithUnsafe(),
		),
		goldmark.WithExtensions(
			extension.GFM,
			extension.NewLinkify(
				extension.WithLinkifyAllowedProtocols([]string{"http:", "https:"}),
			),
			highlighting.NewHighlighting(
				highlighting.WithStyle("monokai"),
				highlighting.WithFormatOptions(chromahtml.WithLineNumbers(false)),
			),
		),
	)

	var buf bytes.Buffer
	if err := markdown.Convert([]byte(processedContent), &buf); err != nil {
		fmt.Println("Error converting markdown to HTML:", err)
		return
	}

	processedFileName := strings.ToLower(strings.ReplaceAll(fileNameWithoutExt, " ", "-"))
	currentPageBacklinks := backlinkMap[processedFileName]
	var backlinks []Backlink
	backlinks = append(backlinks, currentPageBacklinks...)

	fileOutputDir := filepath.Join(outputDir, "pages", processedFileName)
	if err := os.MkdirAll(fileOutputDir, 0o755); err != nil {
		fmt.Println("Error creating file output directory:", err)
		return
	}

	outputFilePath := filepath.Join(fileOutputDir, "index.html")
	f, err := os.Create(outputFilePath)
	if err != nil {
		fmt.Println("Error creating HTML file:", err)
		return
	}
	defer f.Close()

	pageData := PageData{
		Title:     processedFileName,
		Content:   template.HTML(buf.String()),
		Backlinks: backlinks,
	}

	if err := tpl.Execute(f, pageData); err != nil {
		fmt.Println("Error executing template:", err)
		return
	}
}

func CopyDir(src, dst string) error {
	return filepath.Walk(src, func(path string, info fs.FileInfo, err error) error {
		if err != nil {
			return err
		}

		outpath := filepath.Join(dst, strings.TrimPrefix(path, src))

		if info.IsDir() {
			os.MkdirAll(outpath, info.Mode())
			return nil
		}

		if !info.Mode().IsRegular() {
			switch info.Mode().Type() & os.ModeType {
			case os.ModeSymlink:
				link, err := os.Readlink(path)
				if err != nil {
					return err
				}
				return os.Symlink(link, outpath)
			}
			return nil
		}

		in, err := os.Open(path)
		if err != nil {
			return err
		}
		defer in.Close()

		fh, err := os.Create(outpath)
		if err != nil {
			return err
		}
		defer fh.Close()

		fh.Chmod(info.Mode())

		_, err = io.Copy(fh, in)
		return err
	})
}
