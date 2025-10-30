let minimizeClick = () => {
    cpp.win.minimize();
}
let maximizeClick = () => {
    cpp.win.maximize();
}
let restoreClick = () => {
    cpp.win.restore();
}
let closeClick = () => {
    cpp.win.close();
}

window.addEventListener("load", async (event) => {
    let minimize = document.getElementById("minimize")
    minimize.addEventListener("click", minimizeClick);
    let maximize = document.getElementById("maximize")
    maximize.addEventListener("click", maximizeClick);
    let restore = document.getElementById("restore")
    restore.addEventListener("click", restoreClick);
    let close = document.getElementById("close")
    close.addEventListener("click", closeClick);
    let data = await cpp.win.readImg({});
    console.log(data)
})
