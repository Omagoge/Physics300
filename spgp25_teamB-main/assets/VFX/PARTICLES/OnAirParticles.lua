-- Particle System Configuration

return {
    emitters = {
        {
            speed = {
                0,
                0,
            },
            directionRandomness = 0,
            maxParticles = 2600,
            emissionRate = 25,
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            useTexture = true,
            texturePath = "VFX/PARTICLES/VFX_smoke_3.png",
            duration = 5,
            name = "Fire",
            drag = 0.2,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -1.9,
            },
            emissionMode = "continuous",
            startRotation = {
                0,
                360,
            },
            endColor = {
                [3] = 0.33452,
                [1] = 0.33452,
                [2] = 0.33452,
                [4] = 0,
            },
            enabled = true,
            endSize = {
                2.05,
                3.35,
            },
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            coneAngle = 15,
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            randomizeStartColor = false,
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            rotationSpeed = {
                0,
                0,
            },
            startColor = {
                [3] = 0.409253,
                [1] = 0.409253,
                [2] = 0.409253,
                [4] = 1,
            },
            lifetime = {
                0.51,
                0.71,
            },
            startSize = {
                3.75,
                7.65,
            },
            additiveBlending = true,
            shape = "cone",
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            looping = true,
        },
    },
    format = "particle_system",
}
