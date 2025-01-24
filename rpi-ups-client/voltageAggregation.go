package main

type VoltageAggregation struct {
	Sum   float64
	Count int
}

func (va *VoltageAggregation) Add(v float64) {
	va.Sum += v
	va.Count++
}

func (va *VoltageAggregation) Avg() float64 {
	if va.Count == 0 {
		ErrorLog.Println("VoltageAggregation.Avg(): Voltage aggregation is empty")
		return 0
	}
	return va.Sum / float64(va.Count)
}

func (va *VoltageAggregation) Reset() {
	va.Count = 0
	va.Sum = 0
}
