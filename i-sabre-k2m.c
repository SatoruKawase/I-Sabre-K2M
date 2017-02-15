/*
 * ASoC Driver for I-Sabre K2M
 *
 * Author: Satoru Kawase
 *      Copyright 2017 Audiophonics
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <sound/core.h>
#include <sound/soc.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>

#include "i-sabre-codec.h"


static int snd_rpi_i_sabre_k2m_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

static int snd_rpi_i_sabre_k2m_hw_params(
	struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd     = substream->private_data;
	struct snd_soc_dai         *cpu_dai = rtd->cpu_dai;
	int bclk_ratio;

	bclk_ratio = snd_pcm_format_physical_width(
			params_format(params)) * params_channels(params);
	return snd_soc_dai_set_bclk_ratio(cpu_dai, bclk_ratio);
}

/* machine stream operations */
static struct snd_soc_ops snd_rpi_i_sabre_k2m_ops = {
	.hw_params = snd_rpi_i_sabre_k2m_hw_params,
};


static struct snd_soc_dai_link snd_rpi_i_sabre_k2m_dai[] = {
	{
		.name           = "SabreBerry32",
		.stream_name    = "SabreBerry32 DAC",
		.cpu_dai_name   = "bcm2708-i2s.0",
		.codec_dai_name = "sabre9018q2c-dai",
		.platform_name  = "bcm2708-i2s.0",
		.codec_name     = "sabre9018q2c-i2c.1-0048",
		.dai_fmt        = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF
						| SND_SOC_DAIFMT_CBS_CFS,
		.init           = snd_rpi_i_sabre_k2m_init,
		.ops            = &snd_rpi_i_sabre_k2m_ops,
	}
};

/* audio machine driver */
static struct snd_soc_card snd_rpi_i_sabre_k2m = {
	.name      = "SabreBerry32DAC",
	.owner     = THIS_MODULE,
	.dai_link  = snd_rpi_i_sabre_k2m_dai,
	.num_links = ARRAY_SIZE(snd_rpi_i_sabre_k2m_dai)
};


static int snd_rpi_i_sabre_k2m_probe(struct platform_device *pdev)
{
	int ret = 0;

	snd_rpi_i_sabre_k2m.dev = &pdev->dev;
	if (pdev->dev.of_node) {
		struct device_node *i2s_node;
		struct snd_soc_dai_link *dai;

		dai = &snd_rpi_i_sabre_k2m_dai[0];
		i2s_node = of_parse_phandle(pdev->dev.of_node,
							"i2s-controller", 0);
		if (i2s_node) {
			dai->cpu_dai_name     = NULL;
			dai->cpu_of_node      = i2s_node;
			dai->platform_name    = NULL;
			dai->platform_of_node = i2s_node;
		} else {
			dev_err(&pdev->dev,
			    "Property 'i2s-controller' missing or invalid\n");
			return (-EINVAL);
		}

		dai->name        = "I-Sabre K2M";
		dai->stream_name = "I-Sabre K2M DAC";
		dai->dai_fmt     = SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBS_CFS;
	}

	/* Wait for registering codec driver */
	mdelay(50);

	ret = snd_soc_register_card(&snd_rpi_i_sabre_k2m);
	if (ret) {
		dev_err(&pdev->dev,
			"snd_soc_register_card() failed: %d\n", ret);
	}

	return ret;
}

static int snd_rpi_i_sabre_k2m_remove(struct platform_device *pdev)
{
	return snd_soc_unregister_card(&snd_rpi_i_sabre_k2m);
}

static const struct of_device_id snd_rpi_i_sabre_k2m_of_match[] = {
	{ .compatible = "audiophonics,I-Sabre K2M", },
	{}
};
MODULE_DEVICE_TABLE(of, snd_rpi_i_sabre_k2m_of_match);

static struct platform_driver snd_rpi_i_sabre_k2m_driver = {
	.driver = {
		.name           = "snd-rpi-i-sabre-k2m",
		.owner          = THIS_MODULE,
		.of_match_table = snd_rpi_i_sabre_k2m_of_match,
	},
	.probe  = snd_rpi_i_sabre_k2m_probe,
	.remove = snd_rpi_i_sabre_k2m_remove,
};
module_platform_driver(snd_rpi_i_sabre_k2m_driver);

MODULE_DESCRIPTION("ASoC Driver for I-Sabre K2M");
MODULE_AUTHOR("Audiophonics <http://www.audiophonics.fr>");
MODULE_LICENSE("GPL");