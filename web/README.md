# badge wiki
This operates similar to a wiki.

The markdown files in the `pages` dir get generated into html pages.
The `pages` dir should be a flat directory (i.e. no sub dirs).

The syntax is mostly markdown. However, to do an internal link, you should use double square brackets.
e.g.
```markdown
some internal link to the a page called [[badgey bird]]
```

`[[badgey bird]]` would then become a link like this
```html
<a src="/pages/badgey bird/">badgey bird</a>
```

There is also a system of "backlinks".  This just means, at the bottom of a page will there will be a list of pages that link to the current page.

## Dev

Build and run the site with: 
```bash
go run ./cmd/generate/ && go run ./cmd/server/ --dir .dist
```


### Assets
The `assets` directory gets copied into the output dir. This is where images can be stored.  In the markdown files you can reference them as `/assets/someimage.png`.

### HTML Templates
Pages are built with a single html Template `./templates/page.html`.

The main `index.html` is built from `./templates/index.html`

### Extensions

At the end of the day, this site is just html and js, which means we can extend it however we want.

Any js extensions can be created and added to the `js` dir.
At the moment, we only have a few custom html elements.  

Ideally, we should not add a lot of js to this site. So, no need for anything like npm.

