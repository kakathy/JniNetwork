package com.leon.jninetdemo.data

/**
 * @Description:
 * Created By LeonAsha on 2025/3/18
 **/
data class WeatherData(
    val shidu: String,
    val pm25: Double,
    val pm10: Double,
    val quality: String,
    val wendu: String,
    val ganmao: String,
    val forecast: List<Forecast>
)