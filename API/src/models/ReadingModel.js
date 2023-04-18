const mongoose = require('mongoose');
mongoose.set('strict', true);

/**
 * Schema
 */
const readingSchema = new mongoose.Schema({
    name: String,
    Humidity: Number,
    Temperature: Number,
    light: Number,
    DateTime: { type: Date, default: Date.now },
});

module.exports = mongoose.model('Readings', readingSchema);
module.exports = mongoose.model('DANGER', readingSchema);