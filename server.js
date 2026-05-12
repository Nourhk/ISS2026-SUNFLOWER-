// server.js
const express = require("express");
const app = express();
app.use(express.json());
app.use(express.static(__dirname));

let latestData = { ldr: 0 };

app.post("/data", (req, res) => {
  latestData = req.body;
  console.log("Received:", latestData); // This will print the LDR value
  res.send("OK");
});

app.get("/data", (req, res) => {
  res.json(latestData);
});

// Logs endpoint
let logs = [
    { level: "info", message: "Server started.", timestamp: new Date().toISOString() }
];

app.get("/logs", (req, res) => {
    res.json(logs);
});

app.post("/logs", (req, res) => {
    const entry = { level: req.body.level || "info", message: req.body.message, timestamp: new Date().toISOString() };
    logs.push(entry);
    res.json(entry);
});
app.listen(3000, () => {
  console.log("Server running on port 3000");
});     