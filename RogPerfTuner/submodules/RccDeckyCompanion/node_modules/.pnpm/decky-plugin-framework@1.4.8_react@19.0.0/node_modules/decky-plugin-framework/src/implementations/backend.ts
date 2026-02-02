import { addEventListener, call, removeEventListener } from "@decky/api";
import { Logger } from "./logger";

/**
 * The Backend class provides access to plugin Python backend methods
 */
export class Backend {
  /**
   * Private constructor to prevent instantiation
   */
  private constructor() { }

  /**
   * Generic method to make backend calls to Python plugin methods
   * @param name - The name of the method to call
   * @param params - The parameters to pass to the method
   * @returns A Promise of the result type
   */
  public static backend_call<I extends any[], O>(
    name: string,
    ...params: I
  ): Promise<O> {
    return Backend.backend_masked_call(name,[],...params);
  }

  /**
   * Generic method to make backend calls to Python plugin methods
   * @param name - The name of the method to call
   * @param params - The parameters to pass to the method
   * @returns A Promise of the result type
   */
  public static backend_masked_call<I extends any[], O>(
    name: string,
    masked: number[],
    ...params: I
  ): Promise<O> {
    const t0 = Date.now()
    return new Promise<O>((resolve, reject)=>{
      if(name!="log"){
        let paramsStr=""
        if(params){
          params.forEach((val,idx)=>{
            if(masked.includes(idx)){
              paramsStr="**hidden**"
            } else {
              paramsStr=paramsStr+JSON.stringify(val);
            }
            if(idx<params.length-1){
              paramsStr=paramsStr+", "
            }
          })
        }
        Logger.debug("Backend invocation: " + name + "(" + paramsStr + ")")
      }
      call<I, O>(name, ...params).then((value:O)=>{
        if(name!="log"){
          Logger.debug("Backend invocation finished in " + (Date.now() - t0) + " ms with result: " + JSON.stringify(value))
        }
        resolve(value)
      }).catch((reason:any)=>{
        if(name!="log"){
            Logger.error("Backend invocation failed after " + (Date.now() - t0) + " ms with error: " + JSON.stringify(reason))
        }
        reject(reason)
      })
    })
  }

  public static backend_handle(category: string, callBack: (...args: any) => void): () => void {
    const innerCallback = (...args:any) =>{
      Logger.debug("Received backend event for '" + category + "': ", args)
      try{
        callBack(...args);
      }catch(reason:any){
        Logger.error("Error handling backend event for '"+category+"'"+ JSON.stringify(reason))
      }
    }
    Logger.debug("Subscribing backend events for '" + category + "'")
    addEventListener(category, innerCallback)
    return () => { 
      Logger.debug("Unsubscribing backend events for '" + category + "'")
      removeEventListener(category, innerCallback) }
  }
}
