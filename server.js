var express = require('express');

var app = express();

// render the main page
app.get('/', function (request, response) {
	response.sendfile('colors.html');
});

// serve up static resources
app.use(express.static(__dirname + '/public'));

var server = app.listen(3000, function () {
	console.log('Listening on port %d', server.address().port);
});