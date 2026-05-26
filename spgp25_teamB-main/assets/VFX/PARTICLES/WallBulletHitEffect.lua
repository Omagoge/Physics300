-- Particle System Configuration

return {
    format = "particle_system",
    emitters = {
        {
            shape = "cone",
            looping = false,
            useTexture = false,
            bursts = {
                {
                    count = 75,
                    cycleInterval = 0,
                    time = 0,
                },
            },
            directionRandomness = 1,
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            maxParticles = 10000,
            additiveBlending = true,
            drag = 0,
            texturePath = "",
            name = "Sparks",
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
            endSize = {
                0.05,
                0.08,
            },
            endColor = {
                [3] = 0,
                [1] = 1,
                [2] = 0.939502,
                [4] = 0,
            },
            emissionRate = 10,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            startColor = {
                [3] = 0.124175,
                [1] = 0.943061,
                [2] = 0.613758,
                [4] = 1,
            },
            enabled = true,
            rotationSpeed = {
                0,
                0,
            },
            speed = {
                10,
                25,
            },
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            randomizeStartColor = false,
            startSize = {
                0.1,
                0.2,
            },
            emissionMode = "burst",
            coneAngle = 60,
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            lifetime = {
                0.05,
                0.2,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            duration = 5,
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
        },
        {
            shape = "cone",
            looping = false,
            useTexture = false,
            bursts = {
                {
                    count = 25,
                    cycleInterval = 0,
                    time = 0,
                },
            },
            directionRandomness = 1,
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            maxParticles = 10000,
            additiveBlending = true,
            drag = 0,
            texturePath = "",
            name = "Sparks",
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
            endSize = {
                0.01,
                0.02,
            },
            endColor = {
                [3] = 0,
                [1] = 1,
                [2] = 0.5,
                [4] = 0,
            },
            emissionRate = 10,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = -15,
            },
            startColor = {
                [3] = 0.219602,
                [1] = 0.907473,
                [2] = 0.307728,
                [4] = 1,
            },
            enabled = true,
            rotationSpeed = {
                0,
                0,
            },
            speed = {
                5,
                15,
            },
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            randomizeStartColor = false,
            startSize = {
                0.05,
                0.1,
            },
            emissionMode = "burst",
            coneAngle = 20,
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            lifetime = {
                0.3,
                0.8,
            },
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            duration = 5,
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
        },
    },
}
