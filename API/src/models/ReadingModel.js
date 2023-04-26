const mongoose = require('mongoose');
mongoose.set('strict', true);

/**
 * Schema
 */
const readingSchema = new mongoose.Schema({
    Name: String,
    Humidity: Number,
    Temperature: Number,
    Light: Number,
    Safety: String,
    DateTime: { type: Date, default: Date.now },
});

module.exports = mongoose.model('Reading', readingSchema);