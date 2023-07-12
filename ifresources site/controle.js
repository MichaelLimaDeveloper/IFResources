let mqttClient;

window.addEventListener("load", (event) => {
  connectToBroker();

  // const publishBtn = document.querySelector(".publish");
  //  publishBtn.addEventListener("click", function () {
  //   publishMessage();
 // });
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
  });
}

function publishMessage(msg) {
  // const messageInput = document.querySelector("#message");

  const topic = "tcc_ifresources_sendraw_manage";
  const message = msg;

  console.log(`Sending Topic: ${topic}, Message: ${message}`);

  mqttClient.publish(topic, message, {
    qos: 0,
    retain: false,
  });
  // messageInput.value = "";
}

function desligarTrem() {
    publishMessage("P0");
}

function ligarTrem() {
    publishMessage("P1")
}
function aumentarTrem() {
    publishMessage("A1");
}
function diminuirTrem() {
    publishMessage("D1");
}