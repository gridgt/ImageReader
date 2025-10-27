window.addEventListener("load", (event) => {
  let img = document.querySelector("img");
  img.addEventListener("click", async (event) => {
    const file = document.querySelector("#file");
    let filePathObj = await cpp.win.getFilePath({
      _additionalObjects: file.files,
    });
    console.log(filePathObj);
  });

  img.addEventListener("dragover", (e) => {
    e.preventDefault();
  });
  img.addEventListener("drop", async (e) => {
    e.preventDefault();
    let filePathObj = await cpp.win.getFilePath({
      _additionalObjects: e.dataTransfer.files,
    });
    console.log(filePathObj);
  });
});
