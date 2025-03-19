package com.leon.jninetdemo.data

/**
 * @Description:
 * Created By LeonAsha on 2025/3/18
 **/
data class WeatherResponse(
    val message: String,
    val status: Int,
    val date: String,
    val time: String,
    val cityInfo: CityInfo,
    val data: WeatherData,
)