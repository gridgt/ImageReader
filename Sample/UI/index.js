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

let initSelectBtn = () => {
  const selectBtn = document.getElementById("selectBtn");
  selectBtn.addEventListener("click", async () => {
    ipc.once("imageFileSelected", (arg) => {
      document.getElementById("selectedFile").innerHTML = `选中的文件：${arg.filePath}`;
    });
    ipc.invoke("selectImageFile");
  });
};

document.addEventListener("DOMContentLoaded", () => {
  initSelectBtn();
});
