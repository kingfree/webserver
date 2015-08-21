console.time('[Web服务器]');

var http = require('http');
var url = require('url');
var fs = require("fs");
var path = require("path");

var 取内容类型 =
    function(文件名) {
        var 内容类型 = "";

        var 扩展名 = path.extname(文件名);

        switch (扩展名) {
        case ".html":
            内容类型 = "text/html";
            break;
        case ".js":
            内容类型 = "text/javascript";
            break;
        case ".css":
            内容类型 = "text/css";
            break;
        case ".gif":
            内容类型 = "image/gif";
            break;
        case ".jpg":
            内容类型 = "image/jpeg";
            break;
        case ".png":
            内容类型 = "image/png";
            break;
        case ".ico":
            内容类型 = "image/icon";
            break;
        default:
            内容类型 = "application/octet-stream";
        }

        return 内容类型;
    }

var 服务器主函数 =
    function(请求, 资源) {
        var 请求路径 = 请求.url;

        console.log(请求路径);

        var 路径名 = url.parse(请求路径).pathname;

        if (path.extname(路径名) == "") {
            路径名 += "/";
        }

        if (路径名.charAt(路径名.length - 1) == "/") {
            路径名 += "index.html";
        }

        var 文件名 = path.join("./www", 路径名);

        fs.exists(文件名, function(存在) {
            if (存在) {
                资源.writeHead(200, {"Content-Type": 取内容类型(文件名)});
                var stream = fs.createReadStream(文件名,
                                                 {flags: "r", encoding: null});
                stream.on("error", function() {
                    资源.writeHead(404);
                    资源.end("<h1>404 Read Error</h1>");
                });
                stream.pipe(资源);
            } else {
                资源.writeHead(404, {"Content-Type": "text/html"});
                资源.end("<h1>404 Not Found</h1>");
            }
        });

    }

var 服务器 = http.createServer(服务器主函数);

服务器.on("error", function(error) {
    console.log(error);
});


服务器.listen(8124, function() {
    console.log('[Web服务器]: 启动在 http://127.0.0.1:8124/');
    console.timeEnd('[Web服务器]');
});
