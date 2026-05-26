-- Particle System Configuration

return {
    format = "particle_system",
    emitters = {
        {
            coneAngle = 45,
            endColor = {
                [3] = 0.3,
                [1] = 0.3,
                [2] = 0.3,
                [4] = 0,
            },
            directionRandomness = 0.17,
            looping = true,
            emissionMode = "continuous",
            maxParticles = 100,
            emissionRate = 105.5,
            useTexture = true,
            enabled = true,
            rotationSpeed = {
                0,
                0,
            },
            startColor = {
                [3] = 0.761566,
                [1] = 0.761566,
                [2] = 0.761566,
                [4] = 0.6,
            },
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            randomizeStartColor = false,
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            endSize = {
                0.9,
                1.1,
            },
            name = "Smoke",
            lifetime = {
                0.01,
                1.51,
            },
            drag = 0.5,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -0.1,
            },
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            startRotation = {
                0,
                360,
            },
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            shapeSize = {
                [3] = 0,
                [1] = 1.5,
                [2] = 0,
            },
            duration = 5,
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            texturePath = "VFX/PARTICLES/VFX_smoke_2.png",
            shape = "box",
            speed = {
                1,
                1,
            },
            additiveBlending = false,
            startSize = {
                1.66,
                3.06,
            },
        },
        {
            coneAngle = 45,
            endColor = {
                [3] = 0.601424,
                [1] = 1,
                [2] = 0.958866,
                [4] = 0,
            },
            directionRandomness = 0.42,
            looping = true,
            emissionMode = "continuous",
            maxParticles = 200,
            emissionRate = 806.5,
            useTexture = false,
            enabled = true,
            rotationSpeed = {
                0,
                0,
            },
            startColor = {
                [3] = 0.0604982,
                [1] = 1,
                [2] = 0.261104,
                [4] = 1,
            },
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            randomizeStartColor = false,
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            endSize = {
                0.01,
                0.02,
            },
            name = "Sparks",
            lifetime = {
                0.01,
                0.3,
            },
            drag = 0,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            startRotation = {
                0,
                360,
            },
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            shapeSize = {
                [3] = 0,
                [1] = 1.5,
                [2] = 0,
            },
            duration = 5,
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            texturePath = "",
            shape = "box",
            speed = {
                2.5,
                7,
            },
            additiveBlending = true,
            startSize = {
                0.05,
                0.1,
            },
        },
    },
}
