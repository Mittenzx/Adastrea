# Mouse Look Controls - Testing Checklist

## Changes Summary
- **LookSensitivity**: 1.0 → 50.0 (50x increase)
- **LookSensitivityVertical**: 2.0 → 50.0 (25x increase)
- **Max clamp range**: 10.0 → 100.0

## Basic Testing

### Build & Launch
- [ ] Project compiles without errors
- [ ] BP_Battleship loads successfully
- [ ] Game launches without crashes

### Core Functionality Tests
1. **Horizontal Movement**: Move mouse left/right
   - [ ] Ship yaws left/right smoothly
   - [ ] Rotation is visible and responsive (~0.8°/frame at 60fps)

2. **Vertical Movement**: Move mouse up/down
   - [ ] Ship pitches up/down smoothly
   - [ ] Vertical sensitivity matches horizontal

3. **Frame Rate Independence**: Test at different frame rates
   - [ ] Consistent feel at 30fps, 60fps, 120fps

### Edge Cases
- [ ] Rapid mouse movement (no stuttering)
- [ ] Stationary mouse (ship stops rotating)
- [ ] Sensitivity adjustment in Blueprint works

## Troubleshooting

**If rotation appears off-center**: Check mesh pivot in BP_Battleship (mesh component should be centered at 0,0,0 relative to RootComponent)

**If still too slow/fast**: Adjust `LookSensitivity` in Blueprint (range 0.1-100.0)

---

For detailed testing procedures, see `mouse-look-sensitivity-fix.md`
