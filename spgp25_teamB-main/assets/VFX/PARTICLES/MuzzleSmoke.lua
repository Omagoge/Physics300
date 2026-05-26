-- Particle System Configuration

return {
    format = "particle_system",
    emitters = {
        {
            randomizeStartColor = false,
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            maxParticles = 1700,
            texturePath = "j",
            shape = "point",
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            useTexture = false,
            endSize = {
                0.3,
                0.45,
            },
            additiveBlending = true,
            drag = 0.5,
            startColor = {
                [3] = 0.5,
                [1] = 0.5,
                [2] = 0.5,
                [4] = 0.6,
            },
            emissionMode = "continuous",
            name = "Smoke",
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = 0.2,
            },
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            enabled = true,
            directionRandomness = 0.47,
            emissionRate = 55.5,
            endColor = {
                [3] = 0.3,
                [1] = 0.3,
                [2] = 0.3,
                [4] = 0,
            },
            rotationSpeed = {
                0,
                0,
            },
            startRotation = {
                0,
                360,
            },
            speed = {
                0,
                0.3,
            },
            duration = 2,
            shapeSize = {
                [3] = 0.5,
                [1] = 0.5,
                [2] = 0.5,
            },
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            coneAngle = 45,
            startSize = {
                0.05,
                0.2,
            },
            lifetime = {
                2,
                2.7,
            },
            looping = false,
        },
    },
}
