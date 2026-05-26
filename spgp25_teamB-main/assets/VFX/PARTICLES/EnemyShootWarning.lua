-- Particle System Configuration

return {
    emitters = {
        {
            emissionMode = "continuous",
            drag = 5,
            maxParticles = 10000,
            additiveBlending = true,
            speed = {
                1,
                5,
            },
            endColor = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
                [4] = 0,
            },
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            texturePath = "",
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            rotationSpeed = {
                0,
                0,
            },
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            enabled = true,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            coneAngle = 15,
            emissionRate = 100,
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
            randomizeStartColor = false,
            name = "Fire",
            useTexture = false,
            startColor = {
                [3] = 0,
                [1] = 0.882353,
                [2] = 0,
                [4] = 1,
            },
            bursts = {
                {
                    count = 50,
                    cycleInterval = 0.5,
                    time = 0.5,
                },
            },
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            endSize = {
                0.2,
                0.25,
            },
            startSize = {
                0.25,
                0.75,
            },
            looping = true,
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            duration = 5,
            shape = "point",
            lifetime = {
                0.25,
                0.5,
            },
            directionRandomness = 1,
        },
        {
            emissionMode = "continuous",
            drag = 0,
            maxParticles = 10000,
            additiveBlending = true,
            speed = {
                1,
                4,
            },
            endColor = {
                [3] = 0.0980392,
                [1] = 0.588235,
                [2] = 0.196078,
                [4] = 0,
            },
            localOffset = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            texturePath = "",
            direction = {
                [3] = 0,
                [1] = 0,
                [2] = 1,
            },
            rotationSpeed = {
                0,
                0,
            },
            localRotation = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            enabled = true,
            gravity = {
                [3] = 0,
                [1] = 0,
                [2] = 0,
            },
            coneAngle = 45,
            emissionRate = 50,
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
            randomizeStartColor = false,
            name = "Sparks",
            useTexture = false,
            startColor = {
                [3] = 0,
                [1] = 0.705882,
                [2] = 0.0980392,
                [4] = 1,
            },
            bursts = {
                {
                    count = 50,
                    cycleInterval = 0.5,
                    time = 0.5,
                },
            },
            startColorRangeMax = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
                [4] = 1,
            },
            endSize = {
                0.2,
                0.25,
            },
            startSize = {
                0.25,
                -0.75,
            },
            looping = true,
            shapeSize = {
                [3] = 1,
                [1] = 1,
                [2] = 1,
            },
            duration = 5,
            shape = "point",
            lifetime = {
                0.1,
                1,
            },
            directionRandomness = 1,
        },
    },
    format = "particle_system",
}
