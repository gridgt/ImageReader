
let updateOverlay = () => {
    const rect = tarImg.getBoundingClientRect();
    const containerRect = tarImg.parentElement.getBoundingClientRect();
    const left = rect.left - containerRect.left;
    const top = rect.top - containerRect.top;
    overlay.style.left = `${left}px`;
    overlay.style.top = `${top}px`;
    overlay.style.width = `${rect.width}px`;
    overlay.style.height = `${rect.height}px`;
    splitter.style.right = `${textBox.clientWidth - splitter.clientWidth / 2}px`;
}
let onFileDrop = async (e) => {
    e.preventDefault();
    statusBar.innerHTML = "开始识别图像..."
    let startTime = Date.now();
    let data = await cpp.win.readImg({
        $additionalObjects: e.dataTransfer.files
    });
    statusBar.innerHTML = `识别完成，耗时：${Date.now()-startTime}毫秒`
    tarImg.setAttribute("src", `$$temp.png?rnd=${Math.floor(Math.random() * 1000000000)}`);
    tarImg.style.display = "block";
    tip.style.display = "none";
    textBox.style.display = "block";
    console.log(data);
    textBox.innerHTML = "";
    splitter.style.display = "block";
    splitter.style.right = `${textBox.clientWidth-splitter.clientWidth/2}px`;

    data.lines.forEach((item, index) => {
        let dom = document.createElement('div');
        dom.className = "line";
        dom.textContent = item;
        textBox.appendChild(dom);
    })
}

let onMouseMove = (e)=> {
    if (!dragging) return;
    let x = e.clientX;
    if (x < 100) x = 100;
    let textBoxWidth = Math.max(100, document.body.clientWidth - x);
    textBox.style.width = `${textBoxWidth}px`;
    splitter.style.right = `${textBox.clientWidth - splitter.clientWidth / 2}px`;
}

window.addEventListener("load", async (event) => {
    window.tarImg = document.getElementById("tarImg");
    window.overlay = document.getElementById('overlay');
    window.tip = document.getElementById('tip');
    window.textBox = document.getElementById('textBox');
    window.statusBar = document.getElementById('statusBar');
    window.splitter = document.getElementById('splitter');
    window.dragging = false;
    splitter.addEventListener("mousedown", (e) => dragging = true);
    window.addEventListener("mouseup", (e) => dragging = false);
    window.addEventListener("mousemove", onMouseMove);
    tarImg.addEventListener('load', updateOverlay);
    window.addEventListener('resize', updateOverlay);
    document.body.addEventListener('dragover', (e) => e.preventDefault());
    document.body.addEventListener('drop', onFileDrop);
});
