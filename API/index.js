const express = require('express');
const app = express();
const cors = require('cors');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const ReadingModel = require('./src/models/ReadingModel.js');
const ReadingRoutes = require('./src/routes/Reading.js');
const DangerReadingModel = require('./src/models/DangerReadingModel.js');

app.use(cors());
app.use(express.json())
app.use(express.urlencoded({ extended: true }))
app.use("/Reading", ReadingRoutes);
app.use("/DANGER", ReadingRoutes);
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

//connecting to an online database
mongoose.set('strictQuery', true);
mongoose.connect('mongodb://127.0.0.1')
    .then(() => console.log('Connected to MongoDB...'))
    .catch(err => console.error('Could not connect to MongoDB...'));



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

app.get('/Reading', async(req, res) => {
    try {
        const Readings = await ReadingModel.find();
        res.status(200).json(Readings);
    } catch (err) {
        res.status(400).send('Error ' + err);
    }
});

app.get('/DANGER', async(req, res) => {
    try {
        const readings = await DangerReadingModel.find();
        res.status(200).json(readings);
    } catch (error) {
        console.error(error);
        res.status(500).send('Server Error');
    }
});

app.delete('/Reading/:id', async(req, res) => {
    try {
        const result = await ReadingModel.deleteOne({ _id: req.params.id });
        if (result.deletedCount === 0) {
            res.status(404).send('Reading not found');
        } else {
            res.status(200).send('Reading deleted successfully');
        }
    } catch (err) {
        res.status(400).send('Error ' + err);
    }
});

app.delete('/DANGER/:id', async(req, res) => {
    try {
        const result = await DangerReadingModel.deleteOne({ _id: req.params.id });
        if (result.deletedCount === 0) {
            res.status(404).send('Danger reading not found');
        } else {
            res.status(200).send('Danger reading deleted successfully');
        }
    } catch (err) {
        res.status(400).send('Error ' + err);
    }
});

async function Reading(req, res, type) {
    try {
        const name = req.body.name;
        const Humidity = req.body.Humidity;
        const Temperature = req.body.Temperature;
        const Light = req.body.Light;
        const Safety = req.body.Safety;


        const Reading = new ReadingModel({
            name: name,
            Humidity: Humidity,
            Temperature: Temperature,
            Light: Light,
            Safety: Safety,

            DateTime: Date.now(),

        });

        const result = await Reading.save();

        res.status(200).json(result);

    } catch (error) {
        console.error(error);
        res.status(500).send('Server Error');
    }
}

async function DANGER(req, res, type) {
    try {
        const name = req.body.name;
        const Humidity = req.body.Humidity;
        const Temperature = req.body.Temperature;
        const Light = req.body.Light;



        const Reading = new DangerReadingModel({
            name: name,
            Humidity: Humidity,
            Temperature: Temperature,
            Light: Light,


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
    Reading(req, res, "Reading");
});

app.post('/DANGER', async(req, res) => {
    DANGER(req, res, "DANGER");
});


app.listen(3000);