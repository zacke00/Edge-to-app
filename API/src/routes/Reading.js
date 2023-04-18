const express = require('express');
const router = express.Router();
const bodyParser = require('body-parser');
const ReadingModel = require('../models/ReadingModel.js');

router.use(bodyParser.json());
router.use(bodyParser.urlencoded({ extended: false }));
router.use(express.json())
router.use(express.urlencoded({ extended: true }))

router.get('/', async(req, res) => {
    try {
        const Readings = await ReadingModel.find();
        res.status(200).json(Readings);
    } catch (err) {
        res.status(400).send('Error ' + err);
    }
});

router.get('/:name', async(req, res) => {
    const name = req.params.name;
    try {
        const result = await ReadingModel.find({ name: name });
        if (!result || result.length === 0) {
            res.status(400).send('No reading found');
        } else {
            res.status(200).json(result);
        }
    } catch (err) {
        res.status(400).send('Error ' + err);
    }

});


module.exports = router;