
export default function HackCalendar(ctx) {
  const calendars = [
    { calendarId: "hackrva.org_fihosi84pjcuriel6b1ba4qmo8@group.calendar.google.com" },
    {
      calendarId: "1qc2m9vm31ekf46t3dkls0j84e62fihq@import.calendar.google.com",
      color: "#B241D1"
    },
  ];

  return () => {
    const calParams = calendars.map(({ calendarId, color }) => {
      let p = `src=${encodeURIComponent(calendarId)}`;
      if (color) p += `&color=${encodeURIComponent(color)}`;
      return p;
    });

    const qs = calParams.join("&") + "&ctz=America/New_York";
    const src = `https://calendar.google.com/calendar/embed?${qs}`;

    return `
      <div>
        <iframe
          src="${src}"
          style="border:0"
          width="500"
          height="400"
          frameborder="0"
          scrolling="no">
        </iframe>
      </div>
    `;
  };
}

