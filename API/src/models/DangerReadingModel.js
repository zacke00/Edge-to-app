const mongoose = require('mongoose');
mongoose.set('strict', true);
/**
 * Schema
 */
const dangerReadingSchema = new mongoose.Schema({
    Name: String,
    Humidity: Number,
    Temperature: Number,
    Light: Number,
    DateTime: { type: Date, default: Date.now },
});

module.exports = mongoose.model('DANGER', dangerReadingSchema);