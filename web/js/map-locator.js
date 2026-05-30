class MapLocator extends HTMLElement {
	static get observedAttributes() {
		return ["address"];
	}

	connectedCallback() {
		this.render();
	}

	attributeChangedCallback() {
		if (this.isConnected) {
			this.render();
		}
	}

	getMapSrc(addr) {
		const encodedAddress = encodeURIComponent(addr);
		return `https://maps.google.com/maps?width=100%25&height=600&hl=en&q=${encodedAddress}&t=&z=14&ie=UTF8&iwloc=B&output=embed`;
	}

	render() {
		const address = this.getAttribute("address") || "2026A Dabney Rd, Richmond, VA 23230";

		this.innerHTML = `
	    <iframe
	      class="calendar"
	      width="100%"
	      height="300"
	      title="hackrva-google-maps-locator"
	      src="${this.getMapSrc(address)}"
	    ></iframe>
	  `;
	}
}

customElements.define('map-locator', MapLocator);
