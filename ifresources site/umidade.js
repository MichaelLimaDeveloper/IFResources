let mqttClient;

window.addEventListener("load", (event) => {
  connectToBroker();

  // const subscribeBtn = document.querySelector("#subscribe");
  // subscribeBtn.addEventListener("click", function () {
  //   subscribeToTopic();
  // });

  // const unsubscribeBtn = document.querySelector("#unsubscribe");
  // unsubscribeBtn.addEventListener("click", function () {
  //   unsubscribeToTopic();
//  });
  setInterval(subscribeToTopic, 10000);

});


function connectToBroker() {
  const clientId = "client" + Math.random().toString(36).substring(7);

  // Change this to point to your MQTT broker
  const host = "ws://broker.emqx.io:8083/mqtt";

  const options = {
    keepalive: 60,
    clientId: clientId,
    protocolId: "MQTT",
    protocolVersion: 5,
    clean: true,
    reconnectPeriod: 1000,
    connectTimeout: 30 * 1000,
  };

  mqttClient = mqtt.connect(host, options);

  mqttClient.on("error", (err) => {
    console.log("Error: ", err);
    mqttClient.end();
  });

  mqttClient.on("reconnect", () => {
    console.log("Reconnecting...");
  });

  mqttClient.on("connect", () => {
    console.log("Client connected:" + clientId);
  });

  // Received
  mqttClient.on("message", (topic, message, packet) => {
    console.log(
      "Received Message: " + message.toString() + "\nOn topic: " + topic
    );
    const messageTextArea = document.querySelector("#humidade");
    messageTextArea.innerHTML = message += "\r\n"; //tirei o+= e botei só +
  });
}

function subscribeToTopic() {
  const status = document.querySelector("#humidade");
  const topic = "tcc_ifresources_humd";
  console.log(`Subscribing to Topic: ${topic}`);

  mqttClient.subscribe(topic, { qos: 0 });
  // status.style.color = "green";
  // status.value = "SUBSCRIBED";
}

function unsubscribeToTopic() {
  const status = document.querySelector("#status");
  const topic = document.querySelector("#topic").value.trim();
  console.log(`Unsubscribing to Topic: ${topic}`);

  mqttClient.unsubscribe(topic, { qos: 0 });
  // status.style.color = "red";
  // status.value = "UNSUBSCRIBED";
}