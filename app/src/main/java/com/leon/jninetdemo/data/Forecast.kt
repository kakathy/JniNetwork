package com.leon.jninetdemo.data

/**
 * @Description:
 * Created By LeonAsha on 2025/3/18
 **/
data class Forecast(
    val date: String,
    val high: String,
    val low: String,
    val week: String,
    val type: String,
    val aqi: Int,
    val fx: String,
    val fl: String,
    val notice: String
)