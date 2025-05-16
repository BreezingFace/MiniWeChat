const config_module = require('./config');
const Redis = require('ioredis');
const RedisCli = new Redis({
    host: config_module.redis_host,
    port: config_module.redis_port,
    password: config_module.redis_passwd
});

/*
监听错误信息
 */
RedisCli.on('error',function(err){
 console.log('Redis Error:',err);
 RedisCli.quit();
});

/**
 * 根据key获取value
 * @param key
 * @returns {Promise<any>}
 */
 async function GetRedis(key){
    try{
        const result= await RedisCli.get(key);
        if(result==0){
            console.log('result:','<'+result+'>','This key does not exist in redis.');
            return null;
        }
        console.log('result:','<'+result+'>','Get key success.');
        return result; 

    }catch(error){
        console.log('GetRedis error is', error);
        return null
    }
 }
 
/**
 * 根据key查询redis中是否存在key
 * @param {*} key 
 * @returns 
 */
async function QueryRedis(key) {
    try{
        const result = await RedisCli.exists(key)
        //  判断该值是否为空 如果为空返回null
        if (result === 0) {
          console.log('result:<','<'+result+'>','This key is null...');
          return null
        }
        console.log('Result:','<'+result+'>','With this value!...');
        return result
    }catch(error){
        console.log('QueryRedis error is', error);
        return null
    }
  }
 /**
  * 设置key和value，并过期时间
  * @param {*} key 
  * @param {*} value 
  * @param {*} exptime 
  * @returns 
  */
 async function SetRedisExpire(key,value, exptime){
     try{
         // 设置键和值
         await RedisCli.set(key,value)
         // 设置过期时间（以秒为单位）
         await RedisCli.expire(key, exptime);
         return true;
     }catch(error){
         console.log('SetRedisExpire error is', error);
         return false;
     }
 }
 /**
  * 退出函数
  */
 function Quit(){
     RedisCli.quit();
 }
 module.exports = {GetRedis, QueryRedis, Quit, SetRedisExpire,}