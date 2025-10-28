window.addEventListener("load", (event) => {
    let start = document.getElementById("start");
    let end = document.getElementById("end");
    let reading = document.getElementById("reading");

    cpp.win.on("reading", () => {
        start.style.display = "none";
        end.style.display = "none";
        reading.style.display = "flex";
    })
    cpp.win.on("end", () => {
        start.style.display = "none";
        reading.style.display = "none";
        end.style.display = "flex";
        setTimeout(() => {
            start.style.display = "flex";
            reading.style.display = "none";
            end.style.display = "none";  
        }, 6000);
    })
});
