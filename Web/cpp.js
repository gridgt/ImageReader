(() => {
  const eventer = mitt();
  window.addEventListener("load", (event) => {
    window.chrome.webview.addEventListener("message", (event) => {
      const data = event.data;
      if (data.$cbId) {
          eventer.emit(data.$cbId, data);
      } else if (data.$eventName) {
          eventer.emit(data.$eventName, data);
      }
    });
  });
  let callCppMethod = (className, methodName, paramData) => {
    return new Promise((resolve, reject) => {
        let cbId = `cb_${Math.floor(Math.random() * 1000000000)}`;
        eventer.on(cbId, (e) => {
        delete e.$cbId;
        eventer.off(cbId);
        resolve(e);
        });
        paramData.$className = className;
        paramData.$methodName = methodName;
        paramData.$cbId = cbId;
        if (paramData.$additionalObjects) {
            let objs = paramData.$additionalObjects;
            delete paramData.$additionalObjects;
            paramData.$additionalObjectsCount = objs.length;
            window.chrome.webview.postMessageWithAdditionalObjects(paramData, objs);
        } else {
            window.chrome.webview.postMessage(paramData);
        }
    });
  };
  let listenEvent = (className, eventName, callback) => {
    let eName = `${className}_${eventName}`;
    let flag = eventer.all.has(eName) && eventer.all.get(eName)?.length > 0;
    eventer.on(eName, callback);
    if (flag) {
      return;
    }
    return callCppMethod(className, "on", { $eventName: eName });
  };
  let unlistenEvent = (className, eventName, callback) => {
    let eName = `${className}_${eventName}`;
    eventer.off(eName, callback);
    flag = eventer.all.has(eName) && eventer.all.get(eName)?.length > 0;
    if (flag) {
      return;
    }
    return callCppMethod(className, "off", { $eventName: eName });
  };
  let win = {
    minimize: () => {
      return callCppMethod("win", "minimize", {});
    },
    close: () => {
        return callCppMethod("win", "close", {});
    },
    restore: () => {
        return callCppMethod("win", "restore", {});
    },
    maximize: () => {
        return callCppMethod("win", "maximize", {});
    },
    on: (eventName, callback) => {
      return listenEvent("win", eventName, callback);
    },
    off: (eventName, callback) => {
      return unlistenEvent("win", eventName, callback);
    },
    readImg: (param) => {
        return callCppMethod("win", "readImg", param);
    },
  };
  window.cpp = {
    win,
  };
})();
