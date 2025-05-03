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

        #fullscreen-button {
          position: absolute;
          top: 10px;
          left: 10px;
          background-color: #303446;
          color: #c6d0f5;
          border: none;
          padding: 5px 10px;
          cursor: pointer;
          font-family: 'Lucida Console', Monaco, monospace;
        }

        @media (max-width: 600px) {
          #fullscreen-button {
            top: 5px;
            left: 5px;
            padding: 3px 6px;
          }

          #output {
            font-size: 12px;
          }

          canvas.emscripten {
            width: auto;
            height: 80%;
          }
        }
      </style>
      <div class="emscripten">
        <progress value="0" max="100" id="progress" hidden></progress>
      </div>
      <div class="emscripten_border" id="canvas-container">
        <canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()" tabindex="-1"></canvas>
        <button id="fullscreen-button">Fullscreen</button>
      </div>
      ${this.render_console()}
    `;

    this.setupModule();
    this.loadEmscriptenScript();
    this.setupFullscreenButton();
    this.adjustCanvasSize();
    window.addEventListener('resize', this.adjustCanvasSize.bind(this));
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
      }
    };

    canvasElement.addEventListener("webglcontextlost", (e) => {
      alert("WebGL context lost. You will need to reload the page.");
      e.preventDefault();
    });

    window.onerror = (event) => {
      console.error(event);
    };
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
        containerElement.requestFullscreen().catch(err => {
          alert(`Error attempting to enable fullscreen mode: ${err.message} (${err.name})`);
        });
      } else {
        document.exitFullscreen();
      }
    });

    document.addEventListener("fullscreenchange", () => {
      this.resizeCanvasToFullscreen();
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
