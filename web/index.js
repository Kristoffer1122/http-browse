// make a js server that serves index.html over http
const http = require("http");
const fs = require("fs");

const server = http.createServer((request, response) => {
  console.log(request);
  console.log(request.url);
  if (request.url === "/index.html") {
    console.log("Serving index.html");
    // print headers
    console.log(request.headers);
    fs.readFile("index.html", (err, data) => {
      if (err) {
        response.writeHead(500);
        response.end("Error loading index.html");
      } else {
        response.writeHead(200, { "Content-Type": "text/html" });
        response.end(data);
      }
    });
  } else {
    response.writeHead(404);
    response.end("Not found");
  }

  let bytes = new TextEncoder().encode(request).length;
  for (x in request.length) {
    for (i in x) {
      bytes += 1;
    }
  }
  console.log(bytes); // sigma bytes
});

server.listen(3000, () => {
  console.log("Server is listening on port 3000");
});
