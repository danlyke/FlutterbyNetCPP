var http = require('http');

var server = http.createServer(
    function (request, response) {
        response.writeHead(200,
                           {"Content-Type": "text/plain"});
        response.end("Hello World\n");
    });
server.listen(8000);


var server =
    require('node-router').getServer();
server.get("/",
           function (request, response) {
               response.simpleText(200, "Hello World!");
           });
server.listen(8000, "localhost");
