# Design

## Class Diagram

### Legend

```mermaid
flowchart TB
    classDef cls fill:88ff88,color:black

    subgraph Inheritance 
        b[Base Class]:::cls
        d[Derived Class]:::cls
    end
    d --|> b

    subgraph Ownership
       c[Class]:::cls
       o[Object owned by Class]:::cls
    end
    c --> o
```

### Iguana Design

```mermaid
flowchart TB
    classDef cls fill:88ff88,color:black
    classDef algo fill:ff8888,color:black

    subgraph iguana
        Iguana:::cls
    end

    subgraph services
        Algorithm:::cls
        AlgorithmConfig:::cls
        Logger:::cls
    end

    subgraph algorithms
        FiducialCuts:::algo
        FiducialCutsConfig:::algo
        MomentumCorrection:::algo
        MomentumCorrectionConfig:::algo
    end

    Iguana    --> Logger
    Iguana    --> Algorithm
    Algorithm --> Logger
    Algorithm --> AlgorithmConfig

    FiducialCuts       --|> Algorithm
    FiducialCutsConfig --|> AlgorithmConfig
    MomentumCorrection       --|> Algorithm
    MomentumCorrectionConfig --|> AlgorithmConfig
```
