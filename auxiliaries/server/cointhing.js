var express = require('express')
var app = express()
var server = require('http').createServer(app)
var port = 3000

server.listen(port, function () {
    console.log('CoinThing server port: %d', port)
    console.log('Serve path: ' + __dirname + '/../../source/data')
})

app.use(express.static(__dirname + '/../../source/data'))
app.get('/action/setv12', function (req, res) {

    for (const [key, value] of Object.entries(req.query)) {
        console.log(`${key}: ${value}`);
    }

    if (req.query.json) {
        res.type("application/json")
        res.send(req.query.json)
    }
    else {
        res.send('1')
    }
})

app.get('/action/get/name', function (req, res) {
    res.send('CoinThing Server')
})

app.get('/action/get/version', function (req, res) {
    res.send('v0.0.0')
})
