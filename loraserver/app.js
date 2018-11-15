var express = require('express'),
path = require('path'),
bodyParser = require('body-parser'),
cons = require('consolidate'),
dust = require('dustjs-helpers'),
pg = require('pg'),
app = express();
var pool = new pg.Pool({
// DB Connect String
// from owner Loaserver pass 00000000
 connectionString: "postgres://Loraserver:00000000@localhost/Lopicaserver"
});
// Assign Dust Engine to .dust Files
app.engine('dust', cons.dust);

// Set Default Ext .dust
app.set('view engine', 'dust');
app.set('views', __dirname + '/views');

// Set Public Folder
app.use(express.static(path.join(__dirname, 'public')));

// Body Parser Middleware
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

app.get('/', function(req, res){

//connect using created pool
pool.connect(function(err, client, done) {
	if(err) {
	return console.error('error fetching client from pool', err);
	}
client.query('SELECT * FROM public.loradata', function(err, result){
	if(err){
	return console.error('error running query', err);
	}
	res.render('index', {loradata: result.rows});
	done();
	  });
	});
});

// Server
app.listen(3000, function(){
console.log('Server Started On Port 3000');
});