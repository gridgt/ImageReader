let readImage = (file) => {
    if (file) {
        if (!file.type.startsWith('image/')) {
            alert('请选择图像文件！');
            return;
        }
        console.log('选中的图片:', file.name);
    }
    let filePathObj = await cpp.win.getFilePath({
        _additionalObjects: e.dataTransfer.files,
    });
    console.log(filePathObj);
}

window.addEventListener("load", (event) => {
    let body = document.body;
    let fileInput = document.getElementById("file");
    fileInput.addEventListener('change', (e) => {
        const file = e.target.files[0];
        readImage(file);
    });
    body.addEventListener("click", () => {
        fileInput.click();
    });
    body.addEventListener("dragover", (e) => {
        e.preventDefault();
    });
    body.addEventListener("drop", async (e) => {
        e.preventDefault();
        const file = e.target.files[0];
        readImage(file);
    });
});
