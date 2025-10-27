(() => {
  const eventer = mitt();
  window.addEventListener("load", (event) => {
    window.chrome.webview.addEventListener("message", (event) => {
      const data = event.data;
      if (data._cbId) {
        eventer.emit(data._cbId, data);
      } else if (data._eventName) {
        eventer.emit(data._eventName, data);
      }
    });
  });
  let callCppMethod = (className, methodName, paramData) => {
    return new Promise((resolve, reject) => {
      let cbId = `cb_${Math.floor(Math.random() * 1000000000)}`;
      eventer.on(cbId, (e) => {
        delete e._cbId;
        eventer.off(cbId);
        resolve(e);
      });
      paramData._className = className;
      paramData._methodName = methodName;
      paramData._cbId = cbId;
      if (paramData._additionalObjects) {
        let objs = paramData._additionalObjects;
        delete paramData._additionalObjects;
        paramData._additionalObjectsCount = objs.length;
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
    return callCppMethod(className, "on", { _eventName: eName });
  };
  let unlistenEvent = (className, eventName, callback) => {
    let eName = `${className}_${eventName}`;
    let flag = !eventer.all.has(eName) || eventer.all.get(eName)?.length < 1;
    if (flag) return;
    eventer.off(eName, callback);
    flag = eventer.all.has(eName) && eventer.all.get(eName)?.length > 0;
    if (flag) {
      return;
    }
    return callCppMethod(className, "off", { _eventName: eName });
  };
  let win = {
    minimize: () => {
      return callCppMethod("win", "minimize", {});
    },
    on: (eventName, callback) => {
      return listenEvent("win", eventName, callback);
    },
    off: (eventName, callback) => {
      return unlistenEvent("win", eventName, callback);
    },
    getFilePath: (param) => {
        return callCppMethod("win", "getFilePath", param);
    },
  };
  window.cpp = {
    win,
  };
})();
