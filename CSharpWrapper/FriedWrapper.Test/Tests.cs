namespace FriedWrapper.Test;

using FriedWrapper;

public class Tests
{
    [Test]
    public void CanGetSupportedFileVersion()
    {
        Assert.DoesNotThrow(() => FriedApi.GetSupportedFileVersion());
        var version = FriedApi.GetSupportedFileVersion();
        Assert.That(version.StartsWith("FRIED"));
    }

    [Test]
    public void CanFryImageExternalRountrip()
    {
        Assert.That(FriedApi.EncodeImage("test_image.png", "test.fried", 32));
        Assert.That(FriedApi.DecodeImage("test.fried", "test_result.png"));
    }
    
    [Test]
    public void CanFryImage()
    {
        Assert.DoesNotThrow(() =>
        {
            using var image = FriedApi.LoadImage("test_image.png");
            using var friedImage = image.FryImage(FriedFlags.FRIED_DEFAULT | FriedFlags.FRIED_SAVEALPHA, 32);
            using var deFriedImage = friedImage.DeFryImage();
        });
    }
}