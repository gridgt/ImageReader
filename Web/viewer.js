
window.addEventListener("load", async (event) => {
    //let minimize = document.getElementById("minimize")
    //minimize.addEventListener("click", minimizeClick);
    //let maximize = document.getElementById("maximize")
    //maximize.addEventListener("click", maximizeClick);
    //let restore = document.getElementById("restore")
    //restore.addEventListener("click", restoreClick);
    //let close = document.getElementById("close")
    //close.addEventListener("click", closeClick);
    //let data = await cpp.win.readImg({});
    //console.log(data)
    let tarImg = document.getElementById("#tarImg");
    cpp.win.on("imgReady", (data) => {
        tarImg.setAttribute("src", `temp.png?rnd=${Math.floor(Math.random() * 1000000000)}`);
        console.log(data);
    })

    document.body.addEventListener('dragover', (e) => {
        e.preventDefault();
    });
    document.body.addEventListener('drop', async (e) => {
        e.preventDefault();
        cpp.win.readImg({
            $additionalObjects: e.dataTransfer.files
        });
    });
})
