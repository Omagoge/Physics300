-- Particle System Configuration

return {
    emitters = {
        {
            endSize = {
                0.05,
                0.05,
            },
            enabled = true,
            rotationSpeed = {
                0,
                0,
            },
            startRotation = {
                0,
                360,
            },
            duration = 5,
            name = "Sparks",
            emissionRate = 10,
            emissionMode = "burst",
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            additiveBlending = true,
            texturePath = "",
            directionRandomness = 1,
            drag = 0,
            lifetime = {
                0.05,
                0.15,
            },
            useTexture = false,
            shape = "cone",
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            speed = {
                10,
                25,
            },
            coneAngle = 40,
            endColor = {
                [3] = 0,
                [1] = 1,
                [2] = 0.939502,
                [4] = 0,
            },
            randomizeStartColor = false,
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            startSize = {
                0.05,
                0.1,
            },
            bursts = {
                {
                    count = 75,
                    time = 0,
                    cycleInterval = 1.8,
                },
            },
            looping = false,
            maxParticles = 10000,
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            startColor = {
                [3] = 0.124175,
                [1] = 0.943061,
                [2] = 0.613758,
                [4] = 1,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
        },
        {
            endSize = {
                0.01,
                0.02,
            },
            enabled = true,
            rotationSpeed = {
                0,
                0,
            },
            startRotation = {
                0,
                360,
            },
            duration = 5,
            name = "Sparks",
            emissionRate = 10,
            emissionMode = "burst",
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            additiveBlending = true,
            texturePath = "",
            directionRandomness = 1,
            drag = 0,
            lifetime = {
                0.3,
                0.4,
            },
            useTexture = false,
            shape = "cone",
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            speed = {
                5,
                15,
            },
            coneAngle = 20,
            endColor = {
                [3] = 0,
                [1] = 1,
                [2] = 0.5,
                [4] = 0,
            },
            randomizeStartColor = false,
            startColorRangeMin = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 1,
            },
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            startSize = {
                0.05,
                0.1,
            },
            bursts = {
                {
                    count = 50,
                    time = 0,
                    cycleInterval = 1.8,
                },
            },
            looping = false,
            maxParticles = 10000,
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            startColor = {
                [3] = 0.219602,
                [1] = 0.907473,
                [2] = 0.307728,
                [4] = 1,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
        },
    },
    format = "particle_system",
}
