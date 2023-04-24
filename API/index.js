const express = require('express');
const app = express();
const cors = require('cors');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const ReadingModel = require('./src/models/ReadingModel.js');
const ReadingRoutes = require('./src/routes/Reading.js');

app.use(cors());
app.use(express.json())
app.use(express.urlencoded({ extended: true }))
app.use("/Reading", ReadingRoutes);
app.use("/DANGER", ReadingRoutes);
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

//connecting to an online database
mongoose.set('strictQuery', true);
mongoose.connect('mongodb+srv://homezacke:1WZU97SLO0gE2fqS@clusterz.wrcidaz.mongodb.net/?retryWrites=true&w=majority')
    .then(() => { console.log("Connected to DB") })
    .catch((err) => { console.log("Error", err) });



/**
 * Start site when entering the API
 */

app.get('/', function(req, res) {
    res.write('<!DOCTYPE html>')
    res.write('<html style="font-family: Roboto, Arial, sans-serif;">')
    res.write('<head><title>Reading api</title></head>')
    res.write('<body>')
    res.write('<p>Welcome to Readings API</p>')
    res.write('</body>')
    res.write('</html>')
    res.end()
})

/**
 * Post request to add a new reading to the database 
 */

async function handleRequest(req, res, type) {
    try {
        const name = req.body.name;
        const Humidity = req.body.Humidity;
        const Temperature = req.body.Temperature;
        const light = req.body.light;

        const Reading = new ReadingModel({
            name: name,
            Humidity: Humidity,
            Temperature: Temperature,
            light: light,

            DateTime: Date.now(),

        });

        const result = await Reading.save();

        res.status(200).json(result);

    } catch (error) {
        console.error(error);
        res.status(500).send('Server Error');
    }
}

app.post('/Reading', async(req, res) => {
    handleRequest(req, res, "Reading");
});

app.post('/DANGER', async(req, res) => {
    handleRequest(req, res, "DANGER");
});


app.listen(3000);