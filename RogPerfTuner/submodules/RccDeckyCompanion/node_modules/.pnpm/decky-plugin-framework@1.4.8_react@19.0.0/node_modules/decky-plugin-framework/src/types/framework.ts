import { FunctionComponentElement } from "react"

export interface FrameworkCfg{
    translator?: TranslatorCfg      
    system?:SystemCfg
    game?: GameCfg,
    input?:InputCfg
    toast?:ToastCfg
} 

export interface SystemCfg {
    login?: boolean
    suspension?: boolean
    resume?: boolean
    network?: boolean
}

export interface TranslatorCfg {
    translations?: Record<string, Record<string, string>>
}

export interface GameCfg {
    lifeCycle?: boolean
}

export interface InputCfg {
    keyPress?: boolean
    shortcut?: boolean
}

export interface ToastCfg {
    logo?: FunctionComponentElement<any>
}