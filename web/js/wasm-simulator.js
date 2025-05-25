class WasmSimulator extends HTMLElement {
  constructor() {
    super();
  }

  connectedCallback() {
    this.render();
  }

  render_console() {
    if (this.hasAttribute("console")) {
      return `<textarea id="output" rows="8"></textarea>`;
    }
    return ``;
  }

  render() {
    const canvasWidth = this.getAttribute("canvas-width") || "auto";
    const canvasHeight = this.getAttribute("canvas-height") || "80%";

    this.innerHTML = `
	<style>
		.emscripten {
			padding-right: 0;
			margin-left: auto;
			margin-right: auto;
			display: block;
		}

		div.emscripten {
		text-align: center;
		 }

		div.emscripten_border {
			border: 1px solid black;
			position: relative;
		}

		canvas.emscripten {
			border: 0px none;
			background-color: black;
			width: ${canvasWidth};
			height: ${canvasHeight};
		}

		#output {
			/*width: 80%;*/
			margin: 10px 0;
			background-color: #303446;
			color: #c6d0f5;
			font-family: 'Lucida Console', Monaco, monospace;
			outline: none;
			resize: none;
			display: block;
		}

		.button-container {
      display: none;
			position: absolute;
			top: 10px;
			left: 10px;
			grid-template-columns: 1fr 1fr 1fr 1fr;
			gap: 6px;
		}

		.button-container button {
			background-color: #303446;
			color: #c6d0f5;
			border: none;
			padding: 5px 10px;
			cursor: pointer;
			font-family: 'Lucida Console', Monaco, monospace;
		}

		@media (max-width: 600px) {
			.button-container {
				top: 5px;
				left: 5px;
			}

			.button-container button {
				padding: 3px 6px;
				font-size: 12px;
			}
		}

		.volume-control {
			display: flex;
			align-items: center;
			gap: 5px;
			color: #c6d0f5;
			font-family: 'Lucida Console', Monaco, monospace;
		}
		.volume-control input[type="range"] {
			width: 80px;
		}


	</style>
<div class="emscripten_border" id="canvas-container">
	<canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()" tabindex="-1"></canvas>

    <button id="play-button" style="
      position: absolute;
      top: 50%;
      left: 50%;
      transform: translate(-50%,-50%);
      padding: 20px 40px;
      font-size: 24px;
      background: rgba(0,0,0,0.7);
      color: white;
      border: none;
      cursor: pointer;
    ">▶ Play</button>

	<div class="button-container">
		<button id="fullscreen-button">Fullscreen</button>
		<button id="zoom-in-button">Zoom In</button>
		<button id="zoom-out-button">Zoom Out</button>
		<button id="rotate-button">Rotate</button>
		<!--
		<label class="volume-control">
			Volume
			<input type="range" id="volume-slider" min="0" max="1" step="0.01" value="1" />
		</label>
		-->
	</div>
</div>

	${this.render_console()}
    `;

    // this.setupModule();
    // this.loadEmscriptenScript();
    this.querySelector("#play-button").addEventListener("click", () =>
      this.startWasm(),
    );

    this.setupFullscreenButton();
    this.setupButtons();
    // this.setupVolumeControl();
    window.addEventListener("resize", this.adjustCanvasSize.bind(this));
  }

  startWasm() {
    this.querySelector(".button-container").style.display = "grid";
    // hide the play button
    const btn = this.querySelector("#play-button");
    btn.style.display = "none";

    // now safe to init audio & run wasm
    this.setupModule();
    this.loadEmscriptenScript();
    setTimeout(() => this.sendKey("r"), 500);
    setTimeout(() => this.sendKey("r"), 800);
  }

  setupModule() {
    const outputElement = this.querySelector("#output");
    const canvasElement = this.querySelector("#canvas");

    window.Module = {
      print: (...args) => {
        const text = args.join(" ");
        console.log(text);
        if (outputElement) {
          outputElement.value += text + "\n";
          outputElement.scrollTop = outputElement.scrollHeight;
        }
      },
      canvas: canvasElement,
      totalDependencies: 0,
      monitorRunDependencies: (left) => {
        this.totalDependencies = Math.max(this.totalDependencies, left);
      },
      setCanvasSize: (width, height) => {
        canvasElement.width = width;
        canvasElement.height = height;
      },
    };

    canvasElement.addEventListener("webglcontextlost", (e) => {
      alert("WebGL context lost. You will need to reload the page.");
      e.preventDefault();
    });

    window.onerror = (event) => {
      console.error(event);
    };
  }

  sendKey(key) {
    const canvas = this.querySelector("#canvas");

    let code, keyCode;
    if (key === "+") {
      code = "Equal";
      keyCode = 187;
    } else if (key === "-") {
      code = "Minus";
      keyCode = 189;
    } else if (key.length === 1 && /^[a-z]$/i.test(key)) {
      const upper = key.toUpperCase();
      code = `Key${upper}`;
      keyCode = upper.charCodeAt(0);
    } else {
      code = key;
      keyCode = key.length === 1 ? key.charCodeAt(0) : 0;
    }

    const ev = new KeyboardEvent("keydown", {
      key,
      code,
      keyCode,
      which: keyCode,
      bubbles: true,
      cancelable: true,
    });

    canvas.dispatchEvent(ev);
  }

  loadEmscriptenScript() {
    const scriptSrc = this.getAttribute("script") || "badge2025_c.js";
    const script = document.createElement("script");
    script.src = scriptSrc;
    script.async = true;
    script.onload = () => console.log(`${scriptSrc} loaded successfully`);
    script.onerror = () => console.error(`Failed to load ${scriptSrc}`);
    document.body.appendChild(script);
  }

  setupFullscreenButton() {
    const fullscreenButton = this.querySelector("#fullscreen-button");
    const canvasElement = this.querySelector("#canvas");
    const containerElement = this.querySelector("#canvas-container");

    fullscreenButton.addEventListener("click", () => {
      if (!document.fullscreenElement) {
        containerElement.requestFullscreen().catch((err) => {
          alert(
            `Error attempting to enable fullscreen mode: ${err.message} (${err.name})`,
          );
        });
      } else {
        document.exitFullscreen();
      }
    });

    document.addEventListener("fullscreenchange", () => {
      this.resizeCanvasToFullscreen();
    });
  }

  setupButtons() {
    const zoomInButton = this.querySelector("#zoom-in-button");
    const zoomOutButton = this.querySelector("#zoom-out-button");
    const rotateButton = this.querySelector("#rotate-button");
    const canvas = this.querySelector("#canvas");

    zoomInButton.addEventListener("click", () => this.sendKey("+"));
    zoomOutButton.addEventListener("click", () => this.sendKey("-"));
    rotateButton.addEventListener("click", () => this.sendKey("r"));
  }

  setupVolumeControl() {
    const volumeSlider = this.querySelector("#volume-slider");
    window.__wasmVolume = parseFloat(volumeSlider.value);

    volumeSlider.addEventListener("input", () => {
      window.__wasmVolume = parseFloat(volumeSlider.value);
    });
  }

  resizeCanvasToFullscreen() {
    const canvasElement = this.querySelector("#canvas");
    const containerElement = this.querySelector("#canvas-container");

    if (document.fullscreenElement) {
      containerElement.style.width = "auto";
      containerElement.style.height = "100%";
      canvasElement.style.width = "auto";
      canvasElement.style.height = "100%";
    }
  }
}

customElements.define("wasm-simulator", WasmSimulator);
