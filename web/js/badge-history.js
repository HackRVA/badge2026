class GalleryImage extends HTMLElement {
	constructor() {
		super();
		this.attachShadow({ mode: 'open' });
		this.src = this.getAttribute('src') || '';
		this.label = this.getAttribute('label') || '';
	}

	connectedCallback() {
		this.render();
	}

	render() {
		this.shadowRoot.innerHTML = `
          <style>
            figure {
              margin: 0;
              text-align: center;
            }
            img {
              width: 50%;
              height: auto;
              border-radius: 4px;
            }
            figcaption {
              margin-top: 0.5em;
              font-weight: bold;
            }
          </style>
          <figure>
            <figcaption>${this.label}</figcaption>
            <img src="${this.src}" alt="${this.label}" />
          </figure>
        `;
	}
}

customElements.define('gallery-image', GalleryImage);
