-- Particle System Configuration

return {
    emitters = {
        {
            maxParticles = 8600,
            shapeSize = {
                [3] = 0,
                [1] = 1.5,
                [2] = 0,
            },
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            endColor = {
                [3] = 0.729537,
                [1] = 0.975937,
                [2] = 1,
                [4] = 0,
            },
            rotationSpeed = {
                0,
                0,
            },
            name = "Sparks",
            additiveBlending = true,
            shape = "box",
            endSize = {
                0.01,
                0.02,
            },
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            emissionMode = "continuous",
            lifetime = {
                0.01,
                0.71,
            },
            looping = true,
            emissionRate = 1000,
            duration = 5,
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            directionRandomness = 0.42,
            startColor = {
                [3] = 0.0604982,
                [1] = 1,
                [2] = 0.261104,
                [4] = 1,
            },
            startSize = {
                0.1,
                0.2,
            },
            startRotation = {
                0,
                360,
            },
            texturePath = "",
            coneAngle = 45,
            useTexture = false,
            enabled = true,
            speed = {
                2.5,
                7,
            },
            randomizeStartColor = false,
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            drag = 0,
        },
    },
    format = "particle_system",
}
