

/* This driver is designed to control the usage of the ADC block between
 * the touchscreen and any other drivers that may need to use it, such as
 * the hwmon driver.
 *
 * Priority will be given to the touchscreen driver, but as this itself is
 * rate limited it should not starve other requests which are processed in
 * order that they are received.
 *
 * Each user registers to get a client block which uniquely identifies it
 * and stores information such as the necessary functions to callback when
 * action is required.
 */


struct adc_client {
	unsigned int		nr_samples;
	int			        result;    
	unsigned int		 prescale;
	unsigned char		 channel;
	void	(*select_cb)(struct adc_client *c, unsigned selected);
	void	(*convert_cb)(struct adc_client *c,
			      unsigned val1, unsigned val2,
			      unsigned *samples_left);    
    void (*pin_config)(void *pin_map,
                    int pin_map_len,
                    void* device);
};

struct adc_device {    
    struct hw_device_t common;    
	struct adc_client	cli;
};

static struct adc_device *adc_dev;

#define adc_dbg(format,...) do { \
co_printf("[ADC] error:"); \
co_printf(format,##__VA_ARGS__); \
} while(0)

static inline void s3c_adc_convert(struct adc_device *adc)
{

}

static inline void s3c_adc_select(struct adc_device *adc,
				  struct adc_client *client)
{

}

static void s3c_adc_dbgshow(struct adc_device *adc)
{

}

static void s3c_adc_try(struct adc_device *adc)
{
	struct adc_client *next = adc->ts_pend;

	if (!next && !list_empty(&adc_pending)) {
		next = list_first_entry(&adc_pending,
					struct adc_client, pend);
		list_del(&next->pend);
	} else
		adc->ts_pend = NULL;

	if (next) {
		adc_dbg(adc, "new client is %p\n", next);
		adc->cur = next;
		s3c_adc_select(adc, next);
		s3c_adc_convert(adc);
		s3c_adc_dbgshow(adc);
	}
}

int s3c_adc_start(struct adc_client *client,
		  unsigned int channel, unsigned int nr_samples)
{
	struct adc_cfg_t cfg;	
	for(uint8_t i=channel_1;i<=channel_2;i++)
    {
    		system_set_port_mux(GPIO_PORT_D, ADC_CHANNEL_PORT[i], FUN_ADC[i]);
    } 
    memset((void*)&cfg, 0, sizeof(cfg));
    cfg.src = ADC_TRANS_SOURCE_PAD;
    cfg.ref_sel = ADC_REFERENCE_AVDD;
	for(uint8_t i=channel_1;i<=channel_2;i++)
    {
    	cfg.channels |= ADC_CHANNEL[i];
    } 	
    cfg.route.pad_to_sample = 1;
    cfg.clk_sel = ADC_SAMPLE_CLK_24M_DIV13;
    cfg.clk_div = 0x3f;
    adc_init(&cfg);
    adc_enable(NULL, NULL, 0);


	return 0;
}
EXPORT_SYMBOL_GPL(s3c_adc_start);

static void s3c_convert_done(struct adc_client *client,
			     unsigned v, unsigned u, unsigned *left)
{
//	client->result = v;
//	wake_up(client->wait);
}

int s3c_adc_read(struct adc_device *pdev, unsigned int ch)
{
	int ret=-1;
    if(pdev == NULL){
        adc_dbg("input dev is null\r\n", next);
        return -1;
    }
	pdev->cli->convert_cb = s3c_convert_done;
	pdev->cli->result = -1;

	ret = s3c_adc_start(pdev->client, ch, 1);
	if (ret < 0)
		goto err;

	pdev->cli->convert_cb = NULL;
	return pdev->cli->result;

err:
	return ret;
}
EXPORT_SYMBOL_GPL(s3c_adc_read);

static void s3c_adc_default_select(struct adc_client *client,
				   unsigned select)
{
}

struct adc_client *s3c_adc_register(struct platform_device *pdev,
					void (*select)(struct adc_client *client,
						       unsigned int selected),
					void (*conv)(struct adc_client *client,
						     unsigned d0, unsigned d1,
						     unsigned *samples_left),
					unsigned int is_ts)
{
	struct adc_client *client;

	WARN_ON(!pdev);

	if (!select)
		select = s3c_adc_default_select;

	if (!pdev)
		return ERR_PTR(-EINVAL);

	client = kzalloc(sizeof(struct adc_client), GFP_KERNEL);
	if (!client) {
		dev_err(&pdev->dev, "no memory for adc client\n");
		return ERR_PTR(-ENOMEM);
	}

	client->pdev = pdev;
	client->is_ts = is_ts;
	client->select_cb = select;
	client->convert_cb = conv;

	return client;
}
EXPORT_SYMBOL_GPL(s3c_adc_register);

void s3c_adc_release(struct adc_client *client)
{
	unsigned long flags;

	spin_lock_irqsave(&adc_dev->lock, flags);

	/* We should really check that nothing is in progress. */
	if (adc_dev->cur == client)
		adc_dev->cur = NULL;
	if (adc_dev->ts_pend == client)
		adc_dev->ts_pend = NULL;
	else {
		struct list_head *p, *n;
		struct adc_client *tmp;

		list_for_each_safe(p, n, &adc_pending) {
			tmp = list_entry(p, struct adc_client, pend);
			if (tmp == client)
				list_del(&tmp->pend);
		}
	}

	if (adc_dev->cur == NULL)
		s3c_adc_try(adc_dev);

	spin_unlock_irqrestore(&adc_dev->lock, flags);
	kfree(client);
}
EXPORT_SYMBOL_GPL(s3c_adc_release);


static int  adc_remove(struct adc_device *pdev)
{

	return 0;
}
void adc_pin_config(void *pin_map,int pin_map_len,void *device)
{
    if(pin_map == NULL || pin_map_len<=0 || device == NULL){
        DEV_ERR("input is error\r\n");
        return;
    }
    struct adc_device*dev=(struct adc_device*)device;
    //gpio_keys_setup_key(dev,pin_map,pin_map_len);
    return;
}

static int adc_open(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    //malloc dev
	struct adc_device *dev = os_malloc(sizeof(struct adc_device));
    memset(dev, 0, sizeof(*dev));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))adc_remove;
    dev->cli.pin_config=adc_pin_config;
//    keys_set_drvdata(dev);
    *device = (struct hw_device_t*)dev;
    return 0;
}


static struct hw_module_methods_t adc_module_methods = {
    .open =  adc_open,
};


const struct hw_module_t hal_module_info_adc = {
    .tag = HARDWARE_MODULE_TAG,       // 规定的tag
    .version_major = 1,
    .version_minor = 0,
    .id = KEYS_HARDWARE_MODULE_ID,  // 模块id
    .name = "Govee adc Module",     // 名称
    .author = "Govee",
    .methods = &adc_module_methods,// 方法
};

