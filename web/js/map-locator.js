export default function MapLocator(ctx) {
  let address = ctx.attributes.address || "2026A Dabney Rd, Richmond, VA 23230";

  function getMapSrc(addr) {
    const encodedAddress = encodeURIComponent(addr);
    return `https://maps.google.com/maps?width=100%25&height=600&hl=en&q=${encodedAddress}&t=&z=14&ie=UTF8&iwloc=B&output=embed`;
  }

  ctx.onAttributeChanged((name, oldValue, newValue) => {
    if (name === "address") {
      address = newValue;
      ctx.update();
    }
  });

  return () => `
    <iframe
      class="calendar"
      width="100%"
      height="300"
      title="hackrva-google-maps-locator"
      src="${getMapSrc(address)}"
    ></iframe>
  `;
}

