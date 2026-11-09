//! Fan curve editor.

use std::collections::BTreeMap;

use serde::Serialize;

use crate::models::hardware::FanCurveData;

#[derive(Debug, Clone, Serialize)]
pub struct FanCurveSet {
    pub current: FanCurveData,
    pub defaults: FanCurveData,
}

#[derive(Debug, Clone, Serialize)]
pub struct FanCurves {
    pub fans: Vec<String>,
    pub selected_profile: String,
    /// profile -> fan -> curves
    pub curves: BTreeMap<String, BTreeMap<String, FanCurveSet>>,
}
