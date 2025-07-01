# NADIA 🎧

NADIA is an open-source real-time spatial audio engine designed for live sound, club systems, and immersive environments. Inspired by systems like L-ISA (L-Acoustics) and Dolby Atmos, but built to be modular, transparent, and free.

# NADIA Development Roadmap & Multi-Speaker Ideas

## Current State
- ✅ Single speaker type positioning (line arrays)
- ✅ Rectangular venue support
- ✅ Basic coverage calculations
- ✅ Array spacing optimization

## Phase 1: Multi-Speaker Support

### Speaker Type System
Create a speaker database/classification system:

```c
typedef enum {
    SPEAKER_LINE_ARRAY,     // K2, d&b Y-series, etc.
    SPEAKER_SUBWOOFER,      // KS28, SB28, cardioid capable
    SPEAKER_FILL,           // X8, 8XT, wide dispersion
    SPEAKER_DELAY,          // Same as line array but positioning logic differs
    SPEAKER_MONITOR,        // Wedges, IEMs (maybe later)
    SPEAKER_UTILITY         // Announce, safety, etc.
} SpeakerType;

typedef struct {
    SpeakerType type;
    char model[64];
    bool cardioid_capable;  // For subs
    float coupling_distance; // How close for coupling effects
    // ... existing speaker specs
} SpeakerSpec;
```

### Array Configuration Types
```c
typedef enum {
    ARRAY_MAIN_LR,          // Left/Right line arrays
    ARRAY_CENTER,           // Center cluster
    ARRAY_SUB_GROUND,       // Ground-stacked subs
    ARRAY_SUB_FLOWN,        // Flown sub arrays
    ARRAY_SUB_CARDIOID,     // Front/back cardioid pairs
    ARRAY_FILL_FRONT,       // Front fills
    ARRAY_DELAY_TOWER       // Delay speakers
} ArrayType;
```

### Bass Array Math Challenges

**Cardioid Sub Arrays:**
- Front speaker: 0° phase
- Rear speaker: 180° phase + physical distance delay
- Calculate optimal spacing for frequency response
- Account for venue boundaries (can't put rear speakers in audience)

**End-Fire Arrays:**
- Linear spacing based on wavelength
- Phase/delay relationships between drivers
- Directivity pattern prediction

**Ground vs. Flown Subs:**
- Ground coupling effects (half-space loading)
- Height impact on coverage
- Integration with main arrays

## Phase 2: Venue Modeling

### CAD-Style Venue Editor
Since real-time optimization isn't critical, you can focus on:

**Simple Polygon Editor:**
- Click to add venue boundary points
- Drag points to reshape
- Add "obstacles" (pillars, bars, sound booth)
- Simple extrusion for height/balconies

**Multi-Level Support:**
- Floor plans as separate layers
- Balcony overhangs
- Sight-line checking (basic ray-casting)
- Different coverage requirements per level

**Venue Templates:**
- Common venue types (club, theater, arena, festival)
- Load/save custom venues
- Import from simple file formats

### Coverage Calculation Updates
```c
typedef struct {
    float x, y, z;           // 3D coordinates
    bool blocked;            // Sight-line obstruction
    int coverage_level;      // Floor/balcony level
    float spl_requirement;   // Target SPL for this area
} CoveragePoint;

typedef struct {
    Polygon boundary;        // Venue outline
    float floor_height;
    float ceiling_height;
    CoveragePoint *points;   // Grid of coverage points
    int num_points;
    Obstacle *obstacles;     // Pillars, bars, etc.
    int num_obstacles;
} VenueLevel;
```

## Phase 3: Advanced Features

### Speaker Interaction Modeling
- **Coupling effects:** When speakers are close enough to couple
- **Interference patterns:** Constructive/destructive zones
- **Comb filtering:** From multiple arrival times
- **Boundary effects:** Wall/ceiling reflections

### System Integration
- **Power calculations:** Total system power requirements
- **Rigging loads:** Weight distribution, safety factors
- **Cable runs:** Signal routing, power distribution
- **Processing requirements:** DSP channel counts, latency budgets

## Implementation Strategy

### Incremental Development
1. **Multi-speaker database:** Add speaker types and specs
2. **Array type logic:** Different positioning rules per array type
3. **Bass array calculator:** Start with simple cardioid pairs
4. **Venue polygon editor:** Basic 2D shape drawing
5. **3D visualization:** Simple wireframe venue + speakers
6. **Multi-level support:** Add balcony/floor layers

### Architecture Considerations
- **Modular design:** Each speaker type has its own positioning module
- **Plugin system:** Easy to add new speaker models/types
- **Configuration files:** Save/load venue + system designs
- **Preview mode:** Visual feedback before finalizing positions

### GUI Design Ideas
- **Main viewport:** 3D venue visualization
- **Speaker palette:** Drag-and-drop different speaker types
- **Properties panel:** Detailed settings for selected speakers/arrays
- **Command console:** Your planned detailed control interface
- **Coverage heatmap:** Visual SPL/coverage overlay

## Technical Challenges to Solve

### Multi-Speaker Math
- Phase relationships between different driver types
- Frequency-dependent directivity patterns
- Cross-array interference calculations
- Optimal spacing algorithms for different configurations

### Venue Complexity
- Polygon clipping for coverage areas
- 3D ray-casting for sight-lines
- Acoustic shadow calculations
- Balcony under-coverage handling

### Performance
- Efficient coverage calculations for complex venues
- Real-time preview updates during speaker positioning
- Memory management for large venue models

## Beyond Core Features

### Export Capabilities
- **Rigging plots:** Technical drawings for installation
- **System schematics:** Signal flow diagrams
- **Coverage reports:** SPL maps, frequency response predictions
- **Equipment lists:** Automated inventory generation

### Integration Possibilities
- **CAD export:** For integration with architectural drawings
- **Show control:** Interface with lighting/video systems
- **Measurement tools:** Import measurement data for verification
- **Simulation accuracy:** Compare predictions with real measurements
