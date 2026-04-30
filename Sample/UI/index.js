let ipc = {
  invoke: (method, args) => {
    return new Promise((resolve, reject) => {
      const id = Math.random().toString(8).substring(2);
      const msg = { id, method, ...args };
      ipc[id] = { resolve, reject };
      window.chrome.webview.postMessage(msg);
    });
  },
  on: (eventName, listener) => {
    if (ipc[eventName]) {
      ipc[eventName].push(listener);
    } else {
      ipc[eventName] = [listener];
    }
  },
  off: (eventName, listener) => {
    if (ipc[eventName]) {
      ipc[eventName] = ipc[eventName].filter((l) => l !== listener);
    }
  },
  once: (eventName, listener) => {
    const onceListener = (arg) => {
      listener(arg);
      ipc.off(eventName, onceListener);
    };
    ipc.on(eventName, onceListener);
  },
};
window.chrome.webview.addEventListener("message", (event) => {
  const msg = event.data;
  if (msg.id && ipc[msg.id]) {
    if (msg.error) {
      ipc[msg.id].reject(msg.error);
    } else {
      ipc[msg.id].resolve(msg.result);
    }
    delete ipc[msg.id];
  } else if (msg.eventName && ipc[msg.eventName]) {
    ipc[msg.eventName].forEach((listener) => listener(msg));
  }
});

let renderOverlay = async (arg) => {
  let overlay = document.getElementById("overlay");
  overlay.innerHTML = "";
  let img = document.getElementById("img");
  img.src = `/LOCAL:${arg.filePath}`;
  img.style.display = "block";
  arg.data.forEach((item, index) => {
    const box = document.createElement("div");
    box.className = "ocr-box";
    box.dataset.index = index;
    const imgRect = img.getBoundingClientRect();
    const scaleX = (imgRect.width / img.naturalWidth) * 1.5;
    const scaleY = (imgRect.height / img.naturalHeight) * 1.5;
    box.style.left = item.x1 * scaleX + "px";
    box.style.top = item.y1 * scaleY + "px";
    box.style.width = (item.x2 - item.x1) * scaleX + "px";
    box.style.height = (item.y2 - item.y1) * scaleY + "px";
    overlay.appendChild(box);
  });
};

let initSelectBtn = () => {
  const selectBtn = document.getElementById("selectBtn");
  selectBtn.addEventListener("click", async () => {
    ipc.once("imageFileSelected", (arg) => {
      document.getElementById("tip").innerHTML = `操作时间：${arg.duration}ms`;
      renderOverlay(arg);
    });
    ipc.invoke("selectImageFile");
  });
};

document.addEventListener("DOMContentLoaded", () => {
  initSelectBtn();
});
