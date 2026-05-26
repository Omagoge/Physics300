-- Particle System Configuration

return {
    emitters = {
        {
            startColor = {
                [3] = 0.761566,
                [1] = 0.761566,
                [2] = 0.761566,
                [4] = 0.6,
            },
            looping = true,
            texturePath = "",
            directionRandomness = 0.17,
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            enabled = true,
            maxParticles = 100,
            additiveBlending = false,
            speed = {
                1,
                1,
            },
            duration = 5,
            useTexture = false,
            emissionMode = "continuous",
            lifetime = {
                0.01,
                1.51,
            },
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -0.1,
            },
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            randomizeStartColor = false,
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
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            startRotation = {
                0,
                360,
            },
            endSize = {
                0.95,
                0.95,
            },
            emissionRate = 105.5,
            drag = 0.5,
            coneAngle = 45,
            startSize = {
                0.01,
                0.16,
            },
            name = "Smoke",
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            shape = "box",
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            shapeSize = {
                [3] = 0,
                [1] = 1.5,
                [2] = 0,
            },
        },
        {
            startColor = {
                [3] = 0.0604982,
                [1] = 1,
                [2] = 0.261104,
                [4] = 1,
            },
            looping = true,
            texturePath = "",
            directionRandomness = 0.42,
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            enabled = true,
            maxParticles = 200,
            additiveBlending = true,
            speed = {
                2.5,
                7,
            },
            duration = 5,
            useTexture = false,
            emissionMode = "continuous",
            lifetime = {
                0.01,
                0.3,
            },
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            randomizeStartColor = false,
            endColor = {
                [3] = 0.601424,
                [1] = 1,
                [2] = 0.958866,
                [4] = 0,
            },
            rotationSpeed = {
                0,
                0,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            startRotation = {
                0,
                360,
            },
            endSize = {
                0.01,
                0.02,
            },
            emissionRate = 806.5,
            drag = 0,
            coneAngle = 45,
            startSize = {
                0.05,
                0.1,
            },
            name = "Sparks",
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            shape = "box",
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            shapeSize = {
                [3] = 0,
                [1] = 1.5,
                [2] = 0,
            },
        },
    },
    format = "particle_system",
}
